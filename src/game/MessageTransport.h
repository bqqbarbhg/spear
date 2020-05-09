#pragma once

#include "ext/bq_websocket.h"
#include "game/server/Message.h"
#include "sf/Box.h"

sf::Box<sv::Message> readMessage(bqws_msg *wsMsg);
void writeMessage(bqws_socket *ws, sv::Message *msg, const sf::Symbol &from, const sf::Symbol &to);
