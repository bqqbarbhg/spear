#pragma once

#include "sf/Array.h"
#include "sf/Symbol.h"

struct ProcessingDesc
{
	int level = 5;
	int threads = 0;
	bool localProcessing = false;
};

struct ProcessingAsset
{
	sf::Symbol name;
	uint32_t tasksDone = 0;
	uint32_t tasksPending = 0;
};

void initializeProcessing(const ProcessingDesc &desc = ProcessingDesc());
void closeProcessing();
bool updateProcessing();

void queryProcessingAssets(sf::Array<ProcessingAsset> &assets);

