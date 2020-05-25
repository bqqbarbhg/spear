#if defined(SP_DEDICATED_SERVER)

#include "ServerMain.h"
#include "ext/sokol/sokol_time.h"
#include "bq_websocket_platform.h"

// TODO: Replace this
#include <thread>
#include <chrono>

ServerMain *server;

int main(int argc, char **argv)
{
	stm_setup();

	bqws_pt_init(NULL);

    int port = 4004;
	server = serverInit(port);

	for (;;) {
		serverUpdate(server);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}

#endif
