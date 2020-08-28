#if defined(SP_DEDICATED_SERVER)

#include "sf/Base.h"
#include "server/Server.h"
#include "ext/sokol/sokol_time.h"
#include "bq_websocket_platform.h"


// TODO: Replace this
#include <thread>
#include <chrono>

sv::Server *server;

int main(int argc, char **argv)
{
	stm_setup();

	bqws_pt_init(NULL);

	sv::ServerOpts opts;
	opts.port = 4004;
	server = sv::serverInit(opts);
	if (!server) {
		char desc[4096];
		bqws_pt_error err;
		if (bqws_pt_get_error(&err)) {
			bqws_pt_get_error_desc(desc, sizeof(desc), &err);
		} else {
			snprintf(desc, sizeof(desc), "Unknown error");
		}
		sf::debugPrintLine("Failed to setup server: %s", desc);

		return 1;
	}

	for (;;) {
		sv::serverUpdate(server);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}

#endif
