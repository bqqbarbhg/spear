#include "MessageTransport.h"

#include "sp/Json.h"

#include "ext/json_input.h"
#include "ext/json_output.h"

sf::Box<sv::Message> readMessage(bqws_msg *wsMsg)
{
	sf::Box<sv::Message> msg;
	jsi_value *value = jsi_parse_memory(wsMsg->data, wsMsg->size, NULL);
	bqws_free_msg(wsMsg);
	if (!sp::readJson(value, msg)) return { };
	jsi_free(value);
	return msg;
}

void writeMessage(bqws_socket *ws, sv::Message *msg, const sf::Symbol &from, const sf::Symbol &to)
{
	jso_stream s;
	jso_init_growable(&s);
	s.pretty = true;
	s.pretty_wrap = 80;

	sp::writeJson(s, msg);

	bqws_send_binary(ws, s.data, s.pos);

	sf::debugPrint("/* %s -> %s */ %.*s\n", from.data, to.data, (int)s.pos, s.data);
}
