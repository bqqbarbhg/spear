#include "game/server/GameState.h"
#include "game/server/Message.h"

#include "ext/bq_websocket.h"
#include "ext/bq_websocket_platform.h"

#include "sf/Array.h"
#include "sf/Mutex.h"
#include "sp/Json.h"
#include "sf/HashMap.h"
#include "sf/HashSet.h"
#include "sf/File.h"

#include "MessageTransport.h"
#include "LocalServer.h"

#include "ext/json_input.h"

#include <time.h>

static sf::Symbol serverName { "Server" };

using UndoChunk = sf::SmallArray<sf::Box<sv::Command>, 4>;

struct Client
{
	sf::Symbol name;
	bqws_socket *ws = nullptr;
	uint32_t playerId;

	sf::Array<UndoChunk> undoList;
	sf::Array<UndoChunk> redoList;
};

struct Session
{
	uint32_t id = 0;
	uint32_t secret = 0;
	sf::Symbol editRoomPath;

	sf::Box<sv::State> state;

	sf::Array<Client> clients;
	sf::Array<sf::Box<sv::Event>> pendingEvents;

	sf::HashMap<sf::Symbol, uint32_t> objectTypes;
	uint32_t entityIdCounter = 0;
	uint32_t objectIdCounter = 0;
};

struct ServerMain
{
	bqws_pt_server *server;
	LocalServer *localServer;
	sf::HashMap<uint32_t, sf::Box<Session>> sessions;
	sf::Array<bqws_socket*> pendingClients;
	uint32_t clientCounter = 0;
	sf::StringBuf dataRoot;

	sf::HashMap<sf::Symbol, uint32_t> roomSessions;
};

static sf::StaticRecursiveMutex g_configMutex;

template <typename T>
static sf::Box<T> loadConfig(sf::String name)
{
	sf::Symbol path = sf::Symbol(name);

	sf::RecursiveMutexGuard mg(g_configMutex);

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

static sf::Box<sv::State> loadRoom(sf::String name)
{
	sf::Symbol path = sf::Symbol(name);

	sf::RecursiveMutexGuard mg(g_configMutex);

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

	sf::Box<sv::State> box;
	if (!sp::readJson(value, box)) return { };

	for (auto &pair : box->objects) {
		sf::Box<sv::GameObject> objBox = loadConfig<sv::GameObject>(pair.val.id);
		if (objBox) {
			pair.val = *objBox;
		}
	}

	return box;
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
			if (/* state->canStandOn(pos) */ true) {
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

static void pushEventRaw(Session &se, sf::Box<sv::Event> event)
{
	se.pendingEvents.push(std::move(event));
}

static void pushEvent(Session &se, sf::Box<sv::Event> event)
{
	se.state->applyEvent(event);
	se.pendingEvents.push(std::move(event));
}

static uint32_t resolveObjectId(Session &se, const sf::Symbol &path)
{
	sv::State *state = se.state;
	auto res = se.objectTypes.insert(path);
	if (res.inserted) {
		sf::Box<sv::GameObject> object = loadConfig<sv::GameObject>(path);
		if (!object) {
			object = sf::box<sv::GameObject>();
		}

		sv::ObjectId id = state->nextObjectId;
		state->nextObjectId += 2;

		res.entry.val = id;

		// Share component pointers between instances
		se.state->objects[id] = *object;

		sf::Box<sv::EventUpdateObject> ev = sf::box<sv::EventUpdateObject>();
		ev->id = id;
		ev->object = *object;
		pushEventRaw(se, std::move(ev));
	}
	return res.entry.val;
}

static void applyCommand(Session &se, sf::Box<sv::Command> command, sf::Array<sf::Box<sv::Command>> &undoList)
{
	sv::State *state = se.state;
	if (sv::CommandAddInstance *cmd = command->as<sv::CommandAddInstance>()) {

		uint32_t id = se.state->nextInstanceId;
		se.state->nextInstanceId += 2;
		uint32_t objectId = resolveObjectId(se, cmd->typePath);

		sf::Box<sv::CommandRemoveInstance> undoCmd = sf::box<sv::CommandRemoveInstance>();
		undoCmd->id = id;
		undoList.push(undoCmd);

		{
			sf::Box<sv::EventUpdateInstance> ev = sf::box<sv::EventUpdateInstance>();
			ev->id = id;
			ev->instance = cmd->instance;
			ev->instance.objectId = objectId;
			pushEvent(se, std::move(ev));
		}

	} else if (sv::CommandUpdateInstance *cmd = command->as<sv::CommandUpdateInstance>()) {

		sv::InstancedObject *instance = se.state->instances.findValue(cmd->id);
		if (instance) {
			sf::Box<sv::CommandUpdateInstance> undoCmd = sf::box<sv::CommandUpdateInstance>();
			undoCmd->id = cmd->id;
			undoCmd->instance = *instance;
			undoList.push(undoCmd);
		} else {
			sf::Box<sv::CommandRemoveInstance> undoCmd = sf::box<sv::CommandRemoveInstance>();
			undoCmd->id = cmd->id;
			undoList.push(undoCmd);
		}

		{
			sf::Box<sv::EventUpdateInstance> ev = sf::box<sv::EventUpdateInstance>();
			ev->id = cmd->id;
			ev->instance = cmd->instance;
			pushEvent(se, std::move(ev));
		}

	} else if (sv::CommandRemoveInstance *cmd = command->as<sv::CommandRemoveInstance>()) {

		sv::InstancedObject *instance = se.state->instances.findValue(cmd->id);
		if (instance) {
			sf::Box<sv::CommandUpdateInstance> undoCmd = sf::box<sv::CommandUpdateInstance>();
			undoCmd->id = cmd->id;
			undoCmd->instance = *instance;
			undoList.push(undoCmd);
		}

		{
			sf::Box<sv::EventRemoveObject> ev = sf::box<sv::EventRemoveObject>();
			ev->id = cmd->id;
			pushEvent(se, std::move(ev));
		}

	} else if (sv::CommandUpdateObject *cmd = command->as<sv::CommandUpdateObject>()) {

		uint32_t objectId = resolveObjectId(se, cmd->typePath);

		sv::GameObject &type = se.state->objects[objectId];
		{
			sf::Box<sv::CommandUpdateObject> undoCmd = sf::box<sv::CommandUpdateObject>();
			undoCmd->typePath = cmd->typePath;
			undoCmd->object = type;
			undoList.push(undoCmd);
		}

		{
			sf::Box<sv::EventUpdateObject> ev = sf::box<sv::EventUpdateObject>();
			ev->id = objectId;
			ev->object = cmd->object;
			pushEvent(se, std::move(ev));
		}

	} else if (sv::CommandLoadObject *cmd = command->as<sv::CommandLoadObject>()) {

		resolveObjectId(se, cmd->typePath);

	} else {
		sf_failf("Unknown command: %u", command->type);
	}
}

static Session *setupSession(ServerMain *s, uint32_t id, uint32_t secret, const sf::Symbol &roomPath)
{
	if (id == 0) {

		// TODO: Real random
		do {
			id = rand();
		} while (id != 0 && s->sessions.find(id));

		sf::Box<Session> &box = s->sessions[id];
		box = sf::box<Session>();
		Session &session = *box;
		session.id = id;
		session.secret = rand();

		session.state = sf::box<sv::State>();

		if (roomPath) {
			session.editRoomPath = roomPath;

			session.state = loadRoom(roomPath);
		}

		return &session;
	}

	auto it = s->sessions.find(id);
	if (it && it->val->secret == secret) return it->val;
	return nullptr;
}

static void joinSession(Session &session, bqws_socket *ws, sv::MessageJoin *m)
{
	Client &client = session.clients.push();
	client.ws = ws;
	client.name = m->name;
	client.playerId = m->playerId;

	{
		sv::MessageLoad load;
		load.state = session.state;
		load.sessionId = session.id;
		load.sessionSecret = session.secret;
		load.editRoomPath = session.editRoomPath;
		writeMessage(ws, &load, serverName, client.name);
	}

	if (!session.editRoomPath) {
#if 0
		uint32_t entityId = session.allocateEntityId();

		sf::Box<sv::CardType> shortsword = loadConfig<sv::CardType>("Server/Cards/Weapon/Shortsword.js");
		sf::Box<sv::CardType> club = loadConfig<sv::CardType>("Server/Cards/Weapon/Club.js");

		auto player = sf::box<sv::Character>();
		player->name = client.name;
		if (time(NULL) % 2 == 0) {
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
		spawn->data->id = entityId;
		pushEvent(session, std::move(spawn));
#endif
	}
}

static void quitSession(Session &session, Client &client)
{
#if 0
	for (uint32_t id : client.playerEntities) {
		auto destroy = sf::box<sv::EventDestroy>();
		destroy->entity = id;
		pushEvent(session, std::move(destroy));
	}
#endif


	session.clients.removeSwapPtr(&client);
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
			Session *maybeSession = setupSession(s, m->sessionId, m->sessionSecret, m->editRoomPath);

			if (maybeSession) {
				joinSession(*maybeSession, ws, m);
				ws = nullptr;
			}
		}

		if (ws) {
			bqws_free_socket(ws);
		}

		s->pendingClients.removeSwap(i--);
	}

	for (auto &pair : s->sessions) {
		Session &session = *pair.val;

		if (session.pendingEvents.size) {

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

				if (auto m = msg->as<sv::MessageJoin>()) {
					Session *maybeSession = setupSession(s, m->sessionId, m->sessionSecret, m->editRoomPath);
					if (maybeSession) {
						quitSession(session, client);
						joinSession(*maybeSession, client.ws, m);
					}
				} else if (auto m = msg->as<sv::MessageAction>()) {
					sv::Action *action = m->action;

#if 0

					sv::Entity *entity = session.state->entities[action->entity];
					sv::Character *chr = entity->as<sv::Character>();

					if (sf::find(chr->players, client.playerId)) {

						sf::StringBuf error;
						sf::SmallArray<sf::Box<sv::Event>, 64> events;
						if (session.state->applyAction(action, events, error)) {
							sv::MessageActionSuccess ok = { };
							writeMessage(client.ws, &ok, serverName, client.name);

							for (sf::Box<sv::Event> &event : events) {
								pushEvent(session, std::move(event));
							}
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

#endif

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

				} else if (auto m = msg->as<sv::MessageMultiCommand>()) {

					UndoChunk undo;
					for (sf::Box<sv::Command> &cmd : m->commands) {
						if (cmd->type != sv::Command::Undo && cmd->type != sv::Command::Redo) {
							applyCommand(session, std::move(cmd), undo);
						}
					}
					client.undoList.push(std::move(undo));
					client.redoList.clear();

				} else if (auto m = msg->as<sv::MessageQueryFiles>()) {

					if (!sf::contains(m->root, ".")) {

						sf::SmallArray<sf::FileInfo, 64> files;
						sf::listFiles(m->root, files);

						sv::MessageQueryFilesResult resMsg;
						resMsg.root = m->root;
						const char *begin = m->root.data;
						for (const char &c : m->root) {
							if (c == '/') begin = &c + 1;
						}
						resMsg.dir.name.append(sf::String(begin, m->root.size - (begin - m->root.data)));
						for (sf::FileInfo &info : files) {
							if (info.isDirectory) {
								sv::QueryDir &dir = resMsg.dir.dirs.push();
								dir.name = info.name;
							} else {
								sv::QueryFile &file = resMsg.dir.files.push();
								file.name = info.name;
							}
						}

						writeMessage(client.ws, &resMsg, serverName, client.name);
					}

				}

			}

			if (bqws_is_closed(client.ws)) {
				quitSession(session, client);
				bqws_free_socket(client.ws);
			}
		}
	}
}
