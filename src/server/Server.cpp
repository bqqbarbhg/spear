#include "Server.h"
#include "ServerState.h"
#include "sf/Box.h"
#include "Message.h"

#include "game/LocalServer.h"

#include "ext/bq_websocket.h"
#include "ext/bq_websocket_platform.h"

namespace sv {

struct Session;
struct Server;

struct Client
{
	Session *session;
	Server *server;
	bqws_socket *ws = nullptr;
	uint32_t lastSentEvent = 0;
};

struct Session
{
	Server *server;

	uint32_t id = 0;
	uint32_t secret = 0;

	uint32_t eventBase = 0;
	sf::Array<sf::Box<Event>> events;
	sf::Box<ServerState> state;

	sf::Array<Client> clients;
};

struct Server
{
	MessageEncoding messageEncoding;
	bqws_pt_server *server;
	LocalServer *localServer;
	sf::HashMap<uint32_t, sf::Box<Session>> sessions;
	sf::Array<bqws_socket*> pendingClients;
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
	sf::Box<Message> msg = decodeMessage(sf::slice(wsMsg->data, wsMsg->size));
	bqws_free_msg(wsMsg);
	return msg;
}

static Session *setupSession(Server *s, uint32_t id, uint32_t secret)
{
	if (id == 0) {

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

		session.state = sf::box<ServerState>();

		session.state->addCharacterToSelect(session.events, sf::Symbol("Game/Character_Templates/Greborg.json"), 1);
		session.state->addCharacterToSelect(session.events, sf::Symbol("Game/Character_Templates/Urist.json"), 1);

		Prop prop;
		prop.prefabName = sf::Symbol("Game/Props/Test/Barrel.json");
		prop.transform.tile = sf::Vec2i(0, 0);
		session.state->addProp(session.events, prop);

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

	client.lastSentEvent = session.eventBase + session.events.size;

	{
		sv::MessageLoad load;
		load.state = session.state;
		load.sessionId = session.id;
		load.sessionSecret = session.secret;
		sendMessage(client, load);
	}
}

static void quitSession(Session &session, Client &client)
{
	session.clients.removeSwapPtr(&client);
	client.session = nullptr;
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
			if (auto m = msg->as<sv::MessageJoin>()) {
				Session *maybeSession = setupSession(session.server, m->sessionId, m->sessionSecret);
				if (maybeSession) {
					bqws_socket *ws = client.ws;
					quitSession(session, client);
					joinSession(*maybeSession, ws, m);
					break;
				}
			}
		}
	}

	static int HACKCOUNT = 0;
	HACKCOUNT++;
	if (HACKCOUNT % 30 == 0) {
		uint32_t id = 101;
		int x = rand() % 4 - 2;
		int y = rand() % 4 - 2;
		PropTransform t = session.state->props.find(id)->transform;
		t.tile = sf::Vec2i(x, y);
		t.visualOffset.y = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
		t.visualRotation.x = ((float)rand() / (float)RAND_MAX) * sf::F_2PI;
		t.visualRotation.y = ((float)rand() / (float)RAND_MAX) * sf::F_2PI;
		t.visualRotation.z = ((float)rand() / (float)RAND_MAX) * sf::F_2PI;
		t.scale = ((float)rand() / (float)RAND_MAX) * 3.0f + 0.25f;
		session.state->moveProp(session.events, id, t);
	}

	if (HACKCOUNT % 100 == 0) {
		Prefab prefab = session.state->prefabs[sf::Symbol("Game/Props/Test/Barrel.json")];
		sf::Box<sv::ModelComponent> newModel = sf::box<sv::ModelComponent>(*prefab.findComponent<sv::ModelComponent>());
		newModel->scale = ((float)rand() / (float)RAND_MAX) * 2.0f + 0.25f;
		prefab.components[0] = newModel;
		session.state->reloadPrefab(session.events, prefab);
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
}

void serverUpdate(Server *s)
{
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
			Session *maybeSession = setupSession(s, m->sessionId, m->sessionSecret);

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
		Session &session = *s->sessions.data[i].val;
		updateSession(session);
	}
}

}
