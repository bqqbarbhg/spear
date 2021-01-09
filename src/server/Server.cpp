#include "Server.h"
#include "ServerState.h"
#include "sf/Box.h"
#include "Message.h"

#include "game/LocalServer.h"

#include "server/EnemyAI.h"

#include "ext/bq_websocket.h"
#include "ext/bq_websocket_platform.h"

#include "sp/Json.h"

#include "sf/File.h"
#include "sf/Sort.h"

#include <time.h>

namespace sv {

struct Session;
struct Server;

struct Client
{
	Session *session;
	Server *server;
	bqws_socket *ws = nullptr;
	uint32_t lastSentEvent = 0;
	uint32_t clientId;

	// Editor
	sf::Array<sf::Array<sf::Box<sv::Edit>>> undoStack;
	sf::Array<sf::Array<sf::Box<sv::Edit>>> redoStack;
};

struct Session
{
	Server *server;

	uint32_t id = 0;
	uint32_t secret = 0;

	uint32_t replayEventBase = ~0u;
	sf::Box<ServerState> replayState;
	sf::Array<sf::Box<Event>> replayEvents;

	uint32_t eventBase = 0;
	sf::Array<sf::Box<Event>> events;
	sf::Box<ServerState> state;

	uint32_t nextClientId = 0;
	sf::Array<Client> clients;

	sf::Symbol editMapPath;
	AiState aiState;

	time_t idleTime = 0;
};

struct Server
{
	MessageEncoding messageEncoding;
	bqws_pt_server *server;
	LocalServer *localServer;
	sf::HashMap<uint32_t, sf::Box<Session>> sessions;
	sf::Array<bqws_socket*> pendingClients;
	sf::HashMap<sf::Symbol, uint32_t> editSessions;
};

Server *serverInit(const ServerOpts &opts)
{
	bqws_pt_listen_opts ptOpts = { };
	ptOpts.port = (uint16_t)opts.port;
	ptOpts.reuse_port = true;

	bqws_pt_server *server = NULL;
	LocalServer *localServer = NULL;
	
	if (opts.port > 0) {
		server = bqws_pt_listen(&ptOpts);
		if (!server) return nullptr;
	} else {
		localServer = localServerInit(opts.port);
	}

	sf::debugPrintLine("Serving at: %u", opts.port);

	Server *s = new Server();
	s->server = server;
	s->localServer = localServer;
	s->messageEncoding = opts.messageEncoding;

	return s;
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

static void sendMessage(Client &client, const sv::Message &msg)
{
	sf::SmallArray<char, 4096> data;
	encodeMessage(data, msg, client.server->messageEncoding);
	bqws_send_binary(client.ws, data.data, data.size);
}

static sf::Box<Message> readMessageConsume(bqws_msg *wsMsg)
{
	MessageDecodingLimits limits;
	sf::Box<Message> msg = decodeMessage(sf::slice(wsMsg->data, wsMsg->size), limits);
	bqws_free_msg(wsMsg);
	return msg;
}

static sf::Box<ServerState> loadState(const sf::Symbol &name)
{
	jsi_args args = { };
	args.dialect.allow_bare_keys = true;
	args.dialect.allow_comments = true;
	args.dialect.allow_control_in_string = true;
	args.dialect.allow_missing_comma = true;
	args.dialect.allow_trailing_comma = true;
	jsi_value *value = jsi_parse_file(name.data, &args);
	if (!value) {
		sf::debugPrintLine("Failed to parse map %s:%u:%u: %s",
			name.data, args.error.line, args.error.column, args.error.description);
		return { };
	}

	sv::SavedMap map;
	if (!sp::readJson(value, map)) {
		jsi_free(value);
		return { };
	}

	jsi_free(value);

	return map.state;
}

static void loadSessionState(Session &session, const sf::Symbol &name)
{
	session.events.clear();
	session.eventBase = 0;
	for (Client &c : session.clients) {
		c.lastSentEvent = 0;
	}

	sf::Box<ServerState> state = loadState(name);
	if (!state) {
		session.state = sf::box<sv::ServerState>();
		return;
	}

	session.state = state;
	state->loadGlobals(session.events);
	state->loadCanonicalPrefabs(session.events);
}

static Session *setupSession(Server *s, uint32_t id, uint32_t secret, const sf::Symbol &editMap)
{
	if (editMap) {
		auto res = s->editSessions.insert(editMap);
		if (res.inserted) {
			// TODO: Real random
			do {
				id = rand();
			} while (id != 0 && s->sessions.find(id));

			sf::Box<Session> &box = s->sessions[id];
			box = sf::box<Session>();
			Session &session = *box;
			session.server = s;
			session.id = id;
			session.secret = rand();
			session.editMapPath = editMap;

			loadSessionState(session, editMap);

			res.entry.val = id;
		}

		id = res.entry.val;

		{
			auto it = s->sessions.find(id);
			if (it) return it->val;
			return nullptr;
		}

	} else if (id == 0) {

		// TODO: Real random
		do {
			id = rand();
		} while (id != 0 && s->sessions.find(id));

		sf::Box<Session> &box = s->sessions[id];
		box = sf::box<Session>();
		Session &session = *box;
		session.server = s;
		session.id = id;
		session.secret = rand();

		session.aiState.rng = sf::Random(rand());

		loadSessionState(session, sf::Symbol("Maps/Castle/Autoload.json"));

#if 0

#if 0
		session.state->addCharacterToSelect(session.events, sf::Symbol("Game/Character_Templates/Greborg.json"), 5);
		session.state->addCharacterToSelect(session.events, sf::Symbol("Game/Character_Templates/Urist.json"), 5);
		session.state->addCharacterToSelect(session.events, sf::Symbol("Game/Character_Templates/Targon.json"), 5);
		session.state->addCharacterToSelect(session.events, sf::Symbol("Game/Character_Templates/Gobo.json"), 5);
#endif
		session.state->addCharacterToSelect(session.events, sf::Symbol("Prefabs/CharacterTemplates/Goblin_Caster.json"), 5);
		session.state->addCharacterToSelect(session.events, sf::Symbol("Prefabs/CharacterTemplates/Dwarf_Melee.json"), 5);
		session.state->addCharacterToSelect(session.events, sf::Symbol("Prefabs/CharacterTemplates/Enemy_Rat.json"), 5);
		session.state->addCharacterToSelect(session.events, sf::Symbol("Prefabs/CharacterTemplates/Enemy_Slime.json"), 5);

		session.state->selectCharacterSpawn(session.events, sf::Symbol("Prefabs/CharacterTemplates/Goblin_Caster.json"), 1);
		session.state->selectCharacterSpawn(session.events, sf::Symbol("Prefabs/CharacterTemplates/Dwarf_Melee.json"), 1);
		session.state->selectCharacterSpawn(session.events, sf::Symbol("Prefabs/CharacterTemplates/Enemy_Rat.json"), 1);
		session.state->selectCharacterSpawn(session.events, sf::Symbol("Prefabs/CharacterTemplates/Enemy_Slime.json"), 1);

#if 0
		session.state->selectCharacterSpawn(session.events, sf::Symbol("Game/Character_Templates/Greborg.json"), 1);
		session.state->selectCharacterSpawn(session.events, sf::Symbol("Game/Character_Templates/Urist.json"), 1);
		session.state->selectCharacterSpawn(session.events, sf::Symbol("Game/Character_Templates/Targon.json"), 1);
		session.state->selectCharacterSpawn(session.events, sf::Symbol("Game/Character_Templates/Greborg.json"), 1);
		session.state->selectCharacterSpawn(session.events, sf::Symbol("Game/Character_Templates/Urist.json"), 1);
		session.state->selectCharacterSpawn(session.events, sf::Symbol("Game/Character_Templates/Targon.json"), 1);
		session.state->selectCharacterSpawn(session.events, sf::Symbol("Game/Character_Templates/Greborg.json"), 1);
		session.state->selectCharacterSpawn(session.events, sf::Symbol("Game/Character_Templates/Urist.json"), 1);
		session.state->selectCharacterSpawn(session.events, sf::Symbol("Game/Character_Templates/Targon.json"), 1);
#endif

#endif

#if 0
		{
			Prop prop;
			prop.prefabName = sf::Symbol("Prefabs/Props/Test/Barrel.json");
			prop.transform.position = sf::Vec2i(0, 0);
			session.state->addProp(session.events, prop);
		}
#endif

#if 0

		for (int32_t y = -20; y < 20; y++)
		for (int32_t x = -20; x < 20; x++)
		{
			if (x == 0 && y == 0) continue;

			Prop prop;
			prop.prefabName = sf::Symbol("Game/Props/Test/Barrel.json");
			prop.transform.tile = sf::Vec2i(x, y) * 3;
			session.state->addProp(session.events, prop);
		}

		// session.state->selectCharacterSpawn(session.events, sf::Symbol("Game/Character_Templates/Greborg.json"), 1);
#endif

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
	client.session = &session;
	client.server = session.server;
	client.clientId = ++session.nextClientId;

	client.lastSentEvent = session.eventBase + session.events.size;

	{
		sv::MessageLoad load;
		load.state = session.state;
		load.sessionId = session.id;
		load.sessionSecret = session.secret;
		load.clientId = client.clientId;
		load.editPath = session.editMapPath;
		sendMessage(client, load);
	}
}

static void quitSession(Session &session, Client &client)
{
	session.clients.removeSwapPtr(&client);
	client.session = nullptr;
}

static void updateBattleState(Session &session)
{
	uint32_t startCharacterId = 0;
	for (Character &chr : session.state->characters) {
		if (!chr.enemy) continue;
		if (updateTargets(session.aiState, chr.id, *session.state)) {
			startCharacterId = chr.id;
		}
	}
	if (startCharacterId) {
		session.state->startBattle(session.events, startCharacterId);
	} else {
		session.state->endBattle(session.events);
	}
}

static void updateSession(Session &session)
{
	for (uint32_t i = 0; i < session.clients.size; i++) {
		Client &client = session.clients[i];

		bqws_update(client.ws);

		if (bqws_is_closed(client.ws)) {
			bqws_socket *ws = client.ws;
			quitSession(session, client);
			bqws_free_socket(ws);
			continue;
		}

		while (bqws_msg *wsMsg = bqws_recv(client.ws)) {
			sf::Box<Message> msg = readMessageConsume(wsMsg);
			if (!msg) continue;

			if (auto m = msg->as<sv::MessageJoin>()) {
				Session *maybeSession = setupSession(session.server, m->sessionId, m->sessionSecret, m->editPath);
				if (maybeSession) {
					bqws_socket *ws = client.ws;
					quitSession(session, client);
					joinSession(*maybeSession, ws, m);
					break;
				}
			} else if (auto m = msg->as<sv::MessageRequestEdit>()) {
				client.redoStack.clear();
				sf::Array<sf::Box<sv::Edit>> undoBundle;
				for (const sv::Edit *edit : m->edits) {
					session.state->applyEdit(session.events, *edit, undoBundle);
				}
				if (undoBundle.size > 0) {
					client.undoStack.push(std::move(undoBundle));
					if (client.undoStack.size > 1024) {
						client.undoStack.removeOrdered(0, 512);
					}
				}
			} else if (auto m = msg->as<sv::MessageRequestEditUndo>()) {
				if (client.undoStack.size > 0) {
					sf::Array<sf::Box<sv::Edit>> redoBundle;
					for (const sv::Edit *edit : client.undoStack.back()) {
						session.state->applyEdit(session.events, *edit, redoBundle);
					}
					client.undoStack.pop();
					if (redoBundle.size > 0) client.redoStack.push(std::move(redoBundle));
					if (client.redoStack.size > 1024) {
						client.redoStack.removeOrdered(0, 512);
					}
				}
			} else if (auto m = msg->as<sv::MessageRequestEditRedo>()) {
				if (client.redoStack.size > 0) {
					sf::Array<sf::Box<sv::Edit>> undoBundle;
					for (const sv::Edit *edit : client.redoStack.back()) {
						session.state->applyEdit(session.events, *edit, undoBundle);
					}
					client.redoStack.pop();
					if (undoBundle.size > 0) client.undoStack.push(std::move(undoBundle));
					if (client.undoStack.size > 1024) {
						client.undoStack.removeOrdered(0, 512);
					}
				}
			} else if (auto m = msg->as<sv::MessageRequestAction>()) {

				session.state->requestAction(session.events, *m->action);

				updateBattleState(session);

			} else if (auto m = msg->as<sv::MessageRequestReplayBegin>()) {

				session.replayState = sf::box<sv::ServerState>(*session.state);
				session.replayEventBase = session.eventBase + session.events.size;

			} else if (auto m = msg->as<sv::MessageRequestReplayReplay>()) {

				if (session.replayEventBase != ~0u) {
					uint32_t offset = session.eventBase - session.replayEventBase;
					session.replayEvents.clear();
					session.replayEvents.push(session.events.slice().drop(offset));
					session.replayEventBase = ~0u;
				}

				PrefabMap replayPrefabs = session.state->prefabs;
				session.events.clear();
				session.events.push(session.replayEvents);
				session.eventBase = 0;
				session.state = session.replayState;
				session.state->prefabs = replayPrefabs;

				for (Client &cl2 : session.clients) {
					sv::MessageLoad load;
					load.state = session.replayState;
					load.sessionId = session.id;
					load.sessionSecret = session.secret;
					load.clientId = cl2.clientId;
					sendMessage(cl2, load);

					cl2.lastSentEvent = 0;
				}

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

					sf::sortBy(resMsg.dir.dirs, [](const sv::QueryDir &dir) { return sf::String(dir.name); });
					sf::sortBy(resMsg.dir.files, [](const sv::QueryFile &file) { return sf::String(file.name); });

					sendMessage(client, resMsg);
				}

			}
		}
	}

	uint32_t maxUpdates = 20;
	bool hasPlayers = false;
	for (Character &chr : session.state->characters) {
		if (!chr.enemy) {
			hasPlayers = true;
			break;
		}
	}
	if (!hasPlayers) {
		maxUpdates = 1;
	}

	if (session.state->inBattle) {
		for (uint32_t i = 0; i < maxUpdates; i++) {
			uint32_t chrId = session.state->turnInfo.characterId;
			bool isEnemy = false;
			if (Character *chr = session.state->characters.find(chrId)) {
				isEnemy = chr->enemy;
			}
			if (!isEnemy) break;

			if (doEnemyActions(session.aiState, session.events, *session.state)) {
				// Did something reasonable, continue on the next "frame"
				break;
			}

			// Didn't finish the turn..
			if (session.state->turnInfo.characterId == chrId) {
				EndTurnAction endTurn = { };
				endTurn.characterId = chrId;
				session.state->requestAction(session.events, endTurn);
			}
		}

		updateBattleState(session);

	} else {
		for (uint32_t i = 0; i < maxUpdates; i++) {
			uint32_t chrId = session.state->turnInfo.characterId;
			bool isEnemy = false;
			if (Character *chr = session.state->characters.find(chrId)) {
				isEnemy = chr->enemy;
			}
			if (!isEnemy) break;

			{
				EndTurnAction endTurn = { };
				endTurn.characterId = chrId;
				session.state->requestAction(session.events, endTurn);
			}
		}
	}

	if (session.state->errors.size > 0) {
		sv::MessageErrorList msg;
		msg.errors = std::move(session.state->errors);

		sf::SmallArray<char, 4096> data;
		encodeMessage(data, msg, session.server->messageEncoding);
		for (Client &client : session.clients) {
			bqws_send_binary(client.ws, data.data, data.size);
		}

		session.state->errors = std::move(msg.errors);
		session.state->errors.clear();
	}

	sf::HashMap<uint32_t, sf::Array<char>> encodedUpdates;
	uint32_t totalEvents = session.eventBase + session.events.size;

	for (uint32_t i = 0; i < session.clients.size; i++) {
		Client &client = session.clients[i];
		if (client.lastSentEvent == totalEvents) continue;
		sf_assert(client.lastSentEvent < totalEvents);
		sf_assert(client.lastSentEvent >= session.eventBase);

		sf::Array<char> &data = encodedUpdates[client.lastSentEvent];
		if (data.size == 0) {
			sv::MessageUpdate msg;
			msg.events.push(session.events.slice().drop(client.lastSentEvent - session.eventBase));
			encodeMessage(data, msg, session.server->messageEncoding);
		}

		bqws_send_binary(client.ws, data.data, data.size);

		client.lastSentEvent = totalEvents;
	}

	// TODO: Keep some events for rewind?
	if (session.replayEventBase == ~0u) {
		session.eventBase += session.events.size;
		session.events.clear();
	}
}

void serverUpdate(Server *s)
{
	time_t currentTime = time(NULL);

	// Accept new clients
	{
		bqws_opts opts = { };
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
		}
	}

	// Wait for `MessageJoin` from pending clients
	for (uint32_t i = 0; i < s->pendingClients.size; i++) {
		bqws_socket *ws = s->pendingClients[i];
		bqws_update(ws);

		bqws_msg *wsMsg = bqws_recv(ws);
		if (!wsMsg) continue;

		sf::Box<sv::Message> msg = readMessageConsume(wsMsg);
		sf_assert(msg);

		if (auto m = msg->as<sv::MessageJoin>()) {
			Session *maybeSession = setupSession(s, m->sessionId, m->sessionSecret, m->editPath);

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

	// Update active sessions
	for (uint32_t i = 0; i < s->sessions.size(); i++) {
		bool remove = false;
		Session &session = *s->sessions.data[i].val;
		updateSession(session);

		if (session.clients.size == 0) {
			if (session.idleTime == 0) {
				session.idleTime = currentTime;
			}
			uint64_t secondsIdled = (uint64_t)(currentTime - session.idleTime);

			if (secondsIdled >= 10*60) {
				remove = true;
			}

		} else {
			session.idleTime = 0;
		}

		if (remove) {
			s->sessions.remove(session.id);
			i--;
		}
	}
}

}
