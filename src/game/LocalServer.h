#pragma once

#include "ext/bq_websocket.h"

struct LocalServer;

LocalServer *localServerInit(int port);
void localServerFree(LocalServer *ls);
bqws_socket *localServerAccept(LocalServer *ls, const bqws_opts *userOpts, const bqws_server_opts *serverOpts);
bqws_socket *localServerConnect(int port, const bqws_opts *opts, const bqws_client_opts *clientOpts);

