#include "LocalServer.h"

#include "sf/Array.h"
#include "sf/HashMap.h"

static sf::HashMap<int, LocalServer*> g_localServers;

struct LocalSocketPair
{
	bqws_socket *client;
	bqws_socket *server;
};

struct LocalServer
{
	int port;
	sf::Array<LocalSocketPair> clients;
	sf::Array<bqws_socket*> newClients;
};

LocalServer *localServerInit(int port)
{
	LocalServer *ls = new LocalServer();
	ls->port = port;
	sf_assert(!g_localServers[port]);
	g_localServers[port] = ls;
	return ls;
}

void localServerFree(LocalServer *ls)
{
	g_localServers.remove(ls->port);
	for (LocalSocketPair &pair : ls->clients) {
		bqws_free_socket(pair.server);
	}

	delete ls;
}

static size_t localSendToClient(void *user, bqws_socket *ws, const void *data, size_t size)
{
	return bqws_read_from((bqws_socket*)user, data, size);
}

static size_t localRecvFromClient(void *user, bqws_socket *ws, void *data, size_t max_size, size_t min_size)
{
	return bqws_write_to((bqws_socket*)user, data, max_size);
}

static void localClientClose(void *user, bqws_socket *ws)
{
	LocalServer *ls = (LocalServer*)user;
	if (!sf::findRemoveSwap(ls->newClients, ws)) {
		for (LocalSocketPair &pair : ls->clients) {
			if (pair.client == ws) {
				bqws_direct_fail(pair.server, BQWS_ERR_UNKNOWN);
				ls->clients.removeSwapPtr(&pair);
				break;
			}
		}
	}
}

bqws_socket *localServerAccept(LocalServer *ls, const bqws_opts *userOpts, const bqws_server_opts *serverOpts)
{
	if (ls->newClients.size > 0) {
		bqws_socket *clientWS = ls->newClients.popValue();

		bqws_opts opts = { };
		if (userOpts) opts = *userOpts;

		opts.io.send_fn = &localSendToClient;
		opts.io.recv_fn = &localRecvFromClient;
		opts.io.user = clientWS;

		bqws_socket *ws = bqws_new_server(&opts, serverOpts);
		bqws_server_accept(ws, NULL);

		ls->clients.push({ clientWS, ws });

		return ws;
	} else {
		return nullptr;
	}
}

bqws_socket *localServerConnect(int port, const bqws_opts *userOpts, const bqws_client_opts *clientOpts)
{
	LocalServer *ls = g_localServers[port];
	sf_assert(ls);

	bqws_opts opts = { };
	if (userOpts) opts = *userOpts;

	opts.io.close_fn = &localClientClose;
	opts.io.user = ls;
	opts.ping_interval = SIZE_MAX;
	opts.ping_response_timeout = SIZE_MAX;
	
	bqws_socket *ws = bqws_new_client(&opts, clientOpts);
	ls->newClients.push(ws);
	return ws;
}
