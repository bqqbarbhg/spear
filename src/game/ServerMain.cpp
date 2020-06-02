#include "game/server/GameState.h"
#include "game/server/Message.h"

#include "ext/bq_websocket.h"
#include "ext/bq_websocket_platform.h"

#include "sf/Array.h"
#include "sf/Mutex.h"
#include "sp/Json.h"
#include "sf/HashMap.h"
#include "sf/HashSet.h"

#include "MessageTransport.h"
#include "LocalServer.h"

#include "ext/json_input.h"

static sf::Symbol serverName { "Server" };

using UndoChunk = sf::SmallArray<sf::Box<sv::Command>, 4>;

struct Client
{
	sf::Symbol name;
	bqws_socket *ws = nullptr;
	sv::EntityId playerEntity;
	uint32_t playerId;

	sf::Array<UndoChunk> undoList;
	sf::Array<UndoChunk> redoList;
};

struct Session
{
	uint32_t secret;

	sf::Box<sv::State> state;

	sf::Array<Client> clients;
	sf::Array<sf::Box<sv::Event>> pendingEvents;

	sf::Array<sv::EntityId> freeEntityIds;
	sf::HashMap<sv::TileType, uint32_t> tileTypes;
	uint32_t entityIdCounter = 0;

	sv::EntityId allocateEntityId()
	{
		if (freeEntityIds.size) {
			return freeEntityIds.popValue();
		} else {
			return ++entityIdCounter;
		}
	}

	void freeEntityId(sv::EntityId id)
	{
		freeEntityIds.push(id);
	}
};

struct ServerMain
{
	bqws_pt_server *server;
	LocalServer *localServer;
	sf::HashMap<uint32_t, Session> sessions;
	sf::Array<bqws_socket*> pendingClients;
	uint32_t clientCounter = 0;
	sf::StringBuf dataRoot;
};

static sf::Mutex g_configMutex;

template <typename T>
static sf::Box<T> loadConfig(sf::String name)
{
	sf::Symbol path = sf::Symbol(name);

	sf::MutexGuard mg(g_configMutex);

	static sf::HashMap<sf::Symbol, sf::Box<T>> cache;
	sf::Box<T> &entry = cache[path];
	if (entry) return entry;

	jsi_args args = { };
	args.dialect.allow_bare_keys = true;
	args.dialect.allow_comments = true;
	args.dialect.allow_control_in_string = true;
	args.dialect.allow_missing_comma = true;
	args.dialect.allow_trailing_comma = true;
	jsi_value *value = jsi_parse_file(path.data, &args);
	if (!value) {
		sf::debugPrint("Failed to parse %s:%u:%u: %s",
			path.data, args.error.line, args.error.column, args.error.description);
		return { };
	}

	if (!sp::readJson(value, entry)) return { };

	entry->id = path;

	return entry;
}

ServerMain *serverInit(int port)
{
	bqws_pt_listen_opts opts = { };
	opts.port = (uint16_t)port;
	opts.reuse_port = true;

	bqws_pt_server *server = NULL;
	LocalServer *localServer = NULL;
	
	if (port > 0) {
		server = bqws_pt_listen(&opts);
		if (!server) return nullptr;
	} else {
		localServer = localServerInit(port);
	}

	ServerMain *s = new ServerMain();
	s->server = server;
	s->localServer = localServer;

	{
		Session &session = s->sessions[1u];
		session.secret = 10;

		session.state = sf::box<sv::State>();

		sv::Map &map = session.state->map;
		map.tileTypes.push();

		{
			sv::TileType &tile = map.tileTypes.push();
			tile.floorName = sf::Symbol("Game/Tiles/Tile_Test.js");
			tile.floor = true;
		}

		{
			sv::TileType &tile = map.tileTypes.push();
			tile.floorName = sf::Symbol("Game/Tiles/floor.js");
			tile.tileName = sf::Symbol("Game/Tiles/wall.js");
			tile.wall = true;
		}

		for (int32_t y = -10; y <= 10; y++)
		for (int32_t x = -10; x <= 10; x++)
		{
			sf::Vec2i v = { x, y };
			map.setTile(v, (rand() % 15 == 0) ? 2 : 1);
		}

		for (int32_t i = -10; i <= 10; i++) {
			map.setTile(sf::Vec2i(i, -10), 2);
			map.setTile(sf::Vec2i(i, +10), 2);
			map.setTile(sf::Vec2i(-10, i), 2);
			map.setTile(sf::Vec2i(+10, i), 2);
		}
	}

	return s;
}

static sf::Vec2i findSpawnPos(sv::State *state, const sf::Vec2i &targetPos)
{
	float bestDist = HUGE_VALF;
	sf::Vec2i bestPos = targetPos;
	for (int32_t radius = 0; radius < 10; radius++) {
		for (int32_t dy = -radius; dy <= +radius; dy++)
		for (int32_t dx = -radius; dx <= +radius; dx++)
		{
			sf::Vec2i pos = targetPos + sf::Vec2i(dx, dy);
			if (state->canStandOn(pos)) {
				float dist = sf::lengthSq(sf::Vec2(pos - targetPos));
				if (dist < bestDist) {
					bestPos = pos;
					bestDist = dist;
				}
			}
		}
	}
	return bestPos;
}

static void wsLog(void *user, bqws_socket *ws, const char *line)
{
	char addr_str[256];
	bqws_pt_address addr = bqws_pt_get_address(ws);
	bqws_pt_format_address(addr_str, sizeof(addr_str), &addr);
	sf::debugPrintLine("%p (%s): %s", ws, addr_str, line);
}

static void wsLogLocal(void *user, bqws_socket *ws, const char *line)
{
	sf::debugPrintLine("%p : %s", ws, line);
}

static sv::TileId resolveTileId(Session &se, const sv::TileType &type)
{
	sv::State *state = se.state;
	auto res = se.tileTypes.insert(type);
	if (res.inserted) {
		uint32_t index = state->map.tileTypes.size;
		state->map.tileTypes.push(type);
		res.entry.val = index;

		sf::Box<sv::EventUpdateTileType> ev = sf::box<sv::EventUpdateTileType>();
		ev->tileType = type;
		ev->index = index;
		se.pendingEvents.push(ev);
	}
	return res.entry.val;
}

static void applyCommand(Session &se, sf::Box<sv::Command> command, sf::Array<sf::Box<sv::Command>> &undoList)
{
	sv::State *state = se.state;
	if (sv::CommandSetTiles *cmd = command->as<sv::CommandSetTiles>()) {
		sv::TileId id = resolveTileId(se, cmd->tileType);

		sf::Box<sv::CommandSetTilesRaw> undoCmd = sf::box<sv::CommandSetTilesRaw>();

		for (const sf::Vec2i &tile : cmd->tiles) {
			sv::RawTileInfo &info = undoCmd->tiles.push();
			info.position = tile;
			info.tileId = state->map.getTile(tile);
		}

		undoList.push(undoCmd);

		sf::HashSet<sf::Vec2i> updatedChunks;
		for (const sf::Vec2i &tile : cmd->tiles) {
			sf::Vec2i chunk = state->map.setTile(tile, id);
			updatedChunks.insert(chunk);
		}

		for (const sf::Vec2i &chunk : updatedChunks) {
			sf::Box<sv::EventUpdateChunk> ev = sf::box<sv::EventUpdateChunk>();
			ev->position = chunk;
			ev->chunk = state->map.chunks[chunk];
			se.pendingEvents.push(ev);
		}

	} else if (sv::CommandSetTilesRaw *cmd = command->as<sv::CommandSetTilesRaw>()) {

		sf::Box<sv::CommandSetTilesRaw> undoCmd = sf::box<sv::CommandSetTilesRaw>();

		for (const sv::RawTileInfo &tile : cmd->tiles) {
			sv::RawTileInfo &info = undoCmd->tiles.push();
			info.position = tile.position;
			info.tileId = state->map.getTile(tile.position);
		}

		undoList.push(undoCmd);

		sf::HashSet<sf::Vec2i> updatedChunks;
		for (const sv::RawTileInfo &tile : cmd->tiles) {
			sf::Vec2i chunk = state->map.setTile(tile.position, tile.tileId);
			updatedChunks.insert(chunk);
		}

		for (const sf::Vec2i &chunk : updatedChunks) {
			sf::Box<sv::EventUpdateChunk> ev = sf::box<sv::EventUpdateChunk>();
			ev->position = chunk;
			ev->chunk = state->map.chunks[chunk];
			se.pendingEvents.push(ev);
		}

	} else {
		sf_failf("Unknown command: %u", command->type);
	}
}

void serverUpdate(ServerMain *s)
{
	// Accept new clients
	{
		uint32_t id = s->clientCounter + 1;
		sf::SmallStringBuf<128> name;
		name.format("Server to Client %u", id);

		bqws_opts opts = { };
		opts.name = name.data;
		bqws_socket *ws = NULL;
		if (s->server) {
			opts.log_fn = &wsLog;
			ws = bqws_pt_accept(s->server, &opts, NULL);
		} else if (s->localServer) {
			opts.log_fn = &wsLogLocal;
			ws = localServerAccept(s->localServer, &opts, NULL);
		}
		if (ws) {
			bqws_server_accept(ws, "spear");
			s->pendingClients.push(ws);
			++s->clientCounter;
		}
	}

	for (uint32_t i = 0; i < s->pendingClients.size; i++) {
		bqws_socket *ws = s->pendingClients[i];
		bqws_update(ws);

		bqws_msg *wsMsg = bqws_recv(ws);
		if (!wsMsg) continue;

		sf::Box<sv::Message> msg = readMessage(wsMsg);
		sf_assert(msg);

		if (auto m = msg->as<sv::MessageJoin>()) {
			auto it = s->sessions.find(m->sessionId);
			Session &session = it->val;
			sf_assert(session.secret == m->sessionSecret);
			Client &client = session.clients.push();
			client.ws = ws;
			client.name = m->name;
			client.playerEntity = session.allocateEntityId();
			client.playerId = m->playerId;

			{
				sv::MessageLoad load;
				load.state = session.state;
				writeMessage(ws, &load, serverName, client.name);
			}

			{
				sf::Box<sv::CardType> shortsword = loadConfig<sv::CardType>("Server/Cards/Weapon/Shortsword.js");
				sf::Box<sv::CardType> club = loadConfig<sv::CardType>("Server/Cards/Weapon/Club.js");

				auto player = sf::box<sv::Character>();
				player->name = client.name;
				if (rand() % 2 == 0) {
					player->model = sf::Symbol("Game/Characters/goblin.js");
				} else {
					player->model = sf::Symbol("Game/Characters/dwarf.js");
				}
				player->position = findSpawnPos(session.state, sf::Vec2i(0, 0));
				player->players.push(client.playerId);
				player->cards.push({ shortsword });
				player->cards.push({ club });

				auto spawn = sf::box<sv::EventSpawn>();
				spawn->data = player;
				spawn->data->id = client.playerEntity;
				session.pendingEvents.push(spawn);
			}

			s->pendingClients.removeSwap(i--);
		}
	}

	for (auto &pair : s->sessions) {
		Session &session = pair.val;

		if (session.pendingEvents.size) {

			for (auto &event : session.pendingEvents) {
				session.state->applyEvent(event);

				if (auto e = event->as<sv::EventDestroy>()) {
					session.freeEntityId(e->entity);
				}
			}

			sv::MessageUpdate update;
			update.events = std::move(session.pendingEvents);

			for (Client &client : session.clients) {
				writeMessage(client.ws, &update, serverName, client.name);
			}

			session.pendingEvents = std::move(update.events);
			session.pendingEvents.clear();
		}

		for (uint32_t i = 0; i < session.clients.size; i++) {
			Client &client = session.clients[i];

			bqws_update(client.ws);

			while (bqws_msg *wsMsg = bqws_recv(client.ws)) {
				auto msg = readMessage(wsMsg);

				if (auto m = msg->as<sv::MessageAction>()) {
					sv::Action *action = m->action;
					sv::Entity *entity = session.state->entities[action->entity];
					sv::Character *chr = entity->as<sv::Character>();

					if (sf::find(chr->players, client.playerId)) {

						sf::StringBuf error;
						sf::SmallArray<sf::Box<sv::Event>, 64> events;
						if (session.state->applyAction(action, events, error)) {
							sv::MessageActionSuccess ok = { };
							writeMessage(client.ws, &ok, serverName, client.name);
							session.pendingEvents.push(events);
						} else {
							sv::MessageActionFailure fail = { };
							fail.description = std::move(error);
							writeMessage(client.ws, &fail, serverName, client.name);
						}

					} else {
						sv::MessageActionFailure fail = { };
						fail.description.format("Player %u cannot control entity %u", client.playerId, action->entity);
						writeMessage(client.ws, &fail, serverName, client.name);
					}
				} else if (auto m = msg->as<sv::MessageCommand>()) {

					if (m->command->type == sv::Command::Undo) {
						if (client.undoList.size > 0) {
							UndoChunk undo;
							for (sf::Box<sv::Command> &cmd : client.undoList.popValue()) {
								applyCommand(session, cmd, undo);
							}
							client.redoList.push(std::move(undo));
						}
					} else if (m->command->type == sv::Command::Redo) {
						if (client.redoList.size > 0) {
							UndoChunk undo;
							for (sf::Box<sv::Command> &cmd : client.redoList.popValue()) {
								applyCommand(session, cmd, undo);
							}
							client.undoList.push(std::move(undo));
						}
					} else {
						UndoChunk undo;
						applyCommand(session, std::move(m->command), undo);
						client.undoList.push(std::move(undo));
						client.redoList.clear();
					}

				}

			}

			if (bqws_is_closed(client.ws)) {
				auto destroy = sf::box<sv::EventDestroy>();
				destroy->entity = client.playerEntity;
				session.pendingEvents.push(destroy);
				session.clients.removeSwap(i--);
			}
		}
	}
}
