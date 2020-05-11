#pragma once

struct ServerMain;

ServerMain *serverInit(int port);
void serverUpdate(ServerMain *s);
