#if defined(SP_DEDICATED_PROCESSOR)

#include "Processing.h"
#include "sf/Base.h"

#include "ext/sokol/sokol_time.h"
#include "ext/sokol/sokol_args.h"

// TODO: Replace this
#include <thread>
#include <chrono>

int main(int argc, char **argv)
{
	sargs_desc desc = { argc, argv };
	desc.max_args = 16;
	desc.buf_size = 16*4096;
	sargs_setup(&desc);

	ProcessingDesc procDesc;

	int arg;
	arg = sargs_find("level");
	if (arg >= 0) {
		procDesc.level = atoi(sargs_value_at(arg));
	}
	arg = sargs_find("threads");
	if (arg >= 0) {
		procDesc.threads = atoi(sargs_value_at(arg));
	}

	stm_setup();

	sf::debugPrintLine("Processing with level=%d, threads=%d", procDesc.level, procDesc.threads);

	initializeProcessing(procDesc);

	while (updateProcessing()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	closeProcessing();

	return 0;
}

#endif
