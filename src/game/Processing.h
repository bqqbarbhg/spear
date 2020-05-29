#pragma once

struct ProcessingDesc
{
	int level = 5;
	int threads = 0;
};

void initializeProcessing(const ProcessingDesc &desc = ProcessingDesc());
void closeProcessing();
bool updateProcessing();

