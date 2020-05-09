#pragma once

#include "sf/Symbol.h"

struct ClientMain;

ClientMain *clientInit(const sf::Symbol &name);
void clientQuit(ClientMain *client);
bool clientUpdate(ClientMain *client);

void clientDoMoveTemp(ClientMain *client);
