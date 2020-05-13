#pragma once

#include "sf/String.h"

namespace sf {

struct Process;

struct ProcessStartOpts
{
	uint64_t affinityMask = 0;
	bool attachStdIo = false;
};

Process *startProcess(sf::String path, sf::Slice<sf::String> args, const ProcessStartOpts &opts={}, sf::StringBuf *pCommandLine=NULL);

bool getProcessCompleted(Process *p, uint32_t *pCode);

void joinProcess(Process *p);
void detachProcess(Process *p);

size_t readProcessStdOut(Process *p, void *dst, size_t size);
size_t readProcessStdErr(Process *p, void *dst, size_t size);

}
