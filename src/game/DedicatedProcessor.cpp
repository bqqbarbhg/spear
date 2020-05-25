#if defined(SP_DEDICATED_PROCESSOR)

#include "Processing.h"

#include "ext/sokol/sokol_time.h"

// TODO: Replace this
#include <thread>
#include <chrono>

int main(int argc, char **argv)
{
	stm_setup();

	initializeProcessing();

	while (updateProcessing()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	closeProcessing();

	return 0;
}

#endif
