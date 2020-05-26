#include "game/server/GameState.h"
#include "game/server/Message.h"

#include "ext/bq_websocket.h"
#include "ext/bq_websocket_platform.h"

#include "sf/Array.h"

#include "MessageTransport.h"

static sf::Symbol serverName { "Server" };

struct Client
{
	sf::Symbol name;
	bqws_socket *ws = nullptr;
	sv::EntityId playerEntity;
	uint32_t playerId;
};

struct Session
{
	uint32_t secret;

	sf::Box<sv::State> state;

	sf::Array<Client> clients;
	sf::Array<sf::Box<sv::Event>> pendingEvents;

	sf::Array<sv::EntityId> freeEntityIds;
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
	sf::HashMap<uint32_t, Session> sessions;
	sf::Array<bqws_socket*> pendingClients;
	uint32_t clientCounter = 0;
};

ServerMain *serverInit(int port)
{
	bqws_pt_listen_opts opts = { };
	opts.port = (uint16_t)port;
	bqws_pt_server *server = bqws_pt_listen(&opts);
	if (!server) return nullptr;

	ServerMain *s = new ServerMain();
	s->server = server;

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

void serverUpdate(ServerMain *s)
{
	// Accept new clients
	{
		uint32_t id = s->clientCounter + 1;
		sf::SmallStringBuf<128> name;
		name.format("Server to Client %u", id);

		bqws_opts opts = { };
		opts.name = name.data;
		opts.log_fn = &wsLog;
		bqws_socket *ws = bqws_pt_accept(s->server, &opts, NULL);
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
				auto player = sf::box<sv::Character>();
				player->name = client.name;
				player->model = sf::Symbol("Game/Characters/human-astro.js");
				player->position = findSpawnPos(session.state, sf::Vec2i(1, -3));
				player->players.push(client.playerId);

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
