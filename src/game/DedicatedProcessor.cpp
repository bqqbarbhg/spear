#if defined(SP_DEDICATED_PROCESSOR)

#include "Processing.h"

#include "ext/sokol/sokol_time.h"
#include "ext/sokol/sokol_args.h"

// TODO: Replace this
#include <thread>
#include <chrono>

int main(int argc, char **argv)
{
	sargs_desc desc = { argc, argv };
	sargs_setup(&desc);

	ProcessingDesc procDesc;

	if (int arg = sargs_find("level")) {
		procDesc.level = atoi(sargs_value_at(arg));
	}
	if (int arg = sargs_find("threads")) {
		procDesc.threads = atoi(sargs_value_at(arg));
	}

	stm_setup();

	initializeProcessing(procDesc);

	while (updateProcessing()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	closeProcessing();

	return 0;
}

#endif
