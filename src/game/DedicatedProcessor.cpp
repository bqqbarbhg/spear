#if defined(SP_DEDICATED_PROCESSOR)

#include "Processing.h"

#include "ext/sokol/sokol_time.h"
#include "ext/sokol/sokol_args.h"

// TODO: Replace this
#include <thread>
#include <chrono>

int main(int argc, char **argv)
{
	sargs_desc desc = { argc, cargv };
	sargs_setup(&desc);

	ProcessingDesc desc;

	if (int arg = sargs_find("level")) {
		desc.level = atoi(sargs_value_at(arg));
	}
	if (int arg = sargs_find("threads")) {
		desc.threads = atoi(sargs_value_at(arg));
	}

	stm_setup();

	initializeProcessing(level);

	while (updateProcessing()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	closeProcessing();

	return 0;
}

#endif
