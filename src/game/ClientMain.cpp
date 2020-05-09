#include "game/client/ClientState.h"
#include "game/server/Message.h"

#include "MessageTransport.h"

#include "ext/bq_websocket.h"
#include "ext/bq_websocket_platform.h"

static sf::Symbol serverName { "Server" };

struct ClientMain
{
	bqws_socket *ws;
	sf::Symbol name;

	sf::Box<sv::State> serverState;
	cl::State clientState;
};

ClientMain *clientInit(const sf::Symbol &name)
{
	ClientMain *c = new ClientMain();
	c->name = name;

	{
		bqws_opts opts = { };
		opts.name = name.data;
		c->ws = bqws_pt_connect("localhost:4004", NULL, &opts, NULL);
	}

	{
		sv::MessageJoin join;
		join.name = name;
		join.sessionId = 1;
		join.sessionSecret = 10;
		writeMessage(c->ws, &join, c->name, serverName);
	}

	return c;
}

void clientQuit(ClientMain *client)
{
	bqws_close(client->ws, BQWS_CLOSE_NORMAL, NULL, 0);
}

bool clientUpdate(ClientMain *c)
{
	if (bqws_is_closed(c->ws)) {
		return true;
	}

	bqws_update(c->ws);

	while (bqws_msg *wsMsg = bqws_recv(c->ws)) {
		sf::Box<sv::Message> msg = readMessage(wsMsg);
		sf_assert(msg);

		if (auto m = msg->as<sv::MessageLoad>()) {
			m->state->refreshEntityTileMap();
			c->serverState = m->state;
			c->clientState.reset(m->state);
		} else if (auto m = msg->as<sv::MessageUpdate>()) {
			for (auto &event : m->events) {
				c->serverState->applyEvent(event);
				c->clientState.applyEvent(event);
			}
		}
	}

	return false;
}
