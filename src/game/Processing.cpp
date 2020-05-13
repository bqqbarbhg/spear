#include "sf/Process.h"

#include "sf/Array.h"
#include "sf/String.h"
#include "sf/File.h"

struct ResourceFile
{
	sf::StringBuf path;
	sf::StringBuf dstName;
	sf::StringBuf tmpPath;
	sf::StringBuf dstPath;
	uint64_t timestamp = 0;
};

struct ProcessRunTask
{
	sf::StringBuf exeName;
	sf::Array<sf::StringBuf> args;
	uint32_t numThreads = 1;
	sf::StringBuf tmpPath;
	sf::StringBuf dstPath;
	sf::ProcessStartOpts startOpts;
};

enum class TaskPriority
{
	Normal,
	Background,
	Count,
};

struct ActiveProcess
{
	sf::Process *process;
	ProcessRunTask task;
	sf::StringBuf commandLine;
};

struct ProcessState
{
	uint32_t maxConcurrentThreads = 4;
	uint32_t activeConcurrentThreads = 0;

	sf::StringBuf dataRoot;
	sf::StringBuf tempRoot;
	sf::StringBuf buildRoot;
	sf::StringBuf toolRoot;

	sf::Array<ResourceFile> resourceFiles;

	sf::Array<ProcessRunTask> taskQueues[(uint32_t)TaskPriority::Count];
	sf::Array<ActiveProcess> activeProcesses;

	uint32_t fileIndex = 0;
};

ProcessState g_processState;

static void findResourcesImp(ProcessState &state, sf::String root, sf::StringBuf &prefix)
{
	sf::Array<sf::FileInfo> files;
	files.reserve(128);

	{
		sf::SmallStringBuf<256> dir;
		sf::appendPath(dir, root, prefix);
		sf::listFiles(dir, files);
	}

	for (sf::FileInfo &file : files) {
		if (file.isDirectory) {
			uint32_t len = prefix.size;
			sf::appendPath(prefix, file.name);
			findResourcesImp(state, root, prefix);
			prefix.resize(len);
			continue;
		}

		ResourceFile &resFile = state.resourceFiles.push();
		sf::appendPath(resFile.path, prefix, file.name);
	}
}

void initializeProcessing()
{
	ProcessState &state = g_processState;

	sf::appendPath(state.dataRoot, "data");
	sf::appendPath(state.tempRoot, "temp", "data");
	sf::appendPath(state.buildRoot, "build", "data");
	sf::appendPath(state.toolRoot, "tool");
#if SF_OS_WINDOWS
	sf::appendPath(state.toolRoot, "win32");
#elif SF_OS_APPLE
	sf::appendPath(state.toolRoot, "macos");
#else
	sf::appendPath(state.toolRoot, "linux");
#endif

	sf::SmallStringBuf<256> prefix;
	findResourcesImp(state, state.dataRoot, prefix);
}

static bool needsUpdate(ProcessState &state, ResourceFile &file, sf::StringBuf &tmpPath, sf::StringBuf &dstPath, sf::String suffix)
{
	dstPath.clear();
	tmpPath.clear();
	sf::appendPath(dstPath, state.buildRoot, file.path);
	dstPath.append(suffix);
	uint64_t dstTime = sf::getFileTimestamp(dstPath);
	if (dstTime < file.timestamp) {
		sf::appendPath(tmpPath, state.tempRoot, file.path);
		tmpPath.append(suffix);
		sf::createDirectories(dstPath);
		sf::createDirectories(tmpPath);
		return true;
	} else {
		return false;
	}
}

static void addRunTask(ProcessState &state, sf::String tmpPath, sf::String dstPath, TaskPriority priority, uint32_t numThreads, sf::String exe, sf::Slice<sf::StringBuf> args, const sf::ProcessStartOpts &opts={})
{
	sf_assert(numThreads > 0);
	sf_assert(numThreads < 10000);
	ProcessRunTask &task = state.taskQueues[(uint32_t)priority].push();
	task.numThreads = numThreads;
	task.tmpPath = tmpPath;
	task.dstPath = dstPath;
	task.startOpts = opts;
	sf::appendPath(task.exeName, state.toolRoot, exe);
#if SF_OS_WINDOWS
	task.exeName.append(".exe");
#endif
	task.args.push(args);
}

static void tryProcessFile(ProcessState &state, ResourceFile &file, const sf::StringBuf &srcPath)
{
	sf::SmallStringBuf<256> tmpPath,dstPath;
	if (sf::endsWith(file.path, ".png")) {

		if (needsUpdate(state, file, tmpPath, dstPath, ".bc1.dds")) {
			sf::Array<sf::StringBuf> args;
			args.push("-color");
			args.push("-bc1");
			args.push("-fast"); // TEMP
			args.push(srcPath);
			args.push(tmpPath);
			addRunTask(state, tmpPath, dstPath, TaskPriority::Normal, 100, "nvcompress", args);
		}

		if (needsUpdate(state, file, tmpPath, dstPath, ".bc7.dds")) {
			sf::Array<sf::StringBuf> args;
			args.push(srcPath);
			args.push(tmpPath);
			addRunTask(state, tmpPath, dstPath, TaskPriority::Normal, 1, "bc7enc", args);
		}

		if (needsUpdate(state, file, tmpPath, dstPath, ".4x4.astc")) {
			sf::Array<sf::StringBuf> args;
			args.push("-cs");
			args.push(srcPath);
			args.push(tmpPath);
			args.push("4x4");
			args.push("-fast");
			args.push("-j");
			args.push("1");
			addRunTask(state, tmpPath, dstPath, TaskPriority::Background, 1, "astcenc", args);
		}

	}
}

static bool allProcessingDone(ProcessState &state)
{
	if (state.activeProcesses.size) return false;
	for (auto &arr : state.taskQueues) {
		if (arr.size) return false;
	}
	return true;
}

void updateProcessing()
{
	ProcessState &state = g_processState;

	for (uint32_t loopI = 0; loopI < 1; loopI++) {
		uint32_t index = state.fileIndex++;
		if (index >= state.resourceFiles.size) {
			if (allProcessingDone(state)) state.fileIndex = 0;
			break;
		}
		ResourceFile &file = state.resourceFiles[index];
		sf::SmallStringBuf<256> path;
		sf::appendPath(path, state.dataRoot, file.path);
		file.timestamp = sf::getFileTimestamp(path);
		tryProcessFile(state, file, path);
	}

	for (uint32_t i = 0; i < state.activeProcesses.size; i++) {
		ActiveProcess &active = state.activeProcesses[i];
		uint32_t exitCode;
		if (!sf::getProcessCompleted(active.process, &exitCode)) continue;

		if (exitCode == 0) {
			sf::replaceFile(active.task.dstPath, active.task.tmpPath);
		} else {
			sf::debugPrintLine("> Failed: %s", active.commandLine.data);
			size_t num;
			char buf[1024];
			while (num = sf::readProcessStdOut(active.process, buf, sizeof(buf))) {
				sf::debugPrint("%.*s", (int)num, buf);
			}
			while (num = sf::readProcessStdErr(active.process, buf, sizeof(buf))) {
				sf::debugPrint("%.*s", (int)num, buf);
			}
		}


		sf::joinProcess(active.process);
		state.activeConcurrentThreads -= active.task.numThreads;
		state.activeProcesses.removeSwap(i--);
	}

	while (state.activeConcurrentThreads < state.maxConcurrentThreads) {

		ProcessRunTask nextTask;
		uint32_t maxThreads = state.activeConcurrentThreads ? state.maxConcurrentThreads - state.activeConcurrentThreads : 10000;
		for (auto &arr : state.taskQueues) {
			if (arr.size > 0) {
				for (uint32_t i = 0; i < arr.size; i++) {
					if (arr[0].numThreads > maxThreads) continue;
					nextTask = std::move(arr[i]);
					arr.removeOrdered(i);
					break;
				}
				if (nextTask.exeName.size) break;
			}
		}

		if (nextTask.exeName.size == 0) break;

		sf::SmallArray<sf::String, 32> args;
		args.reserve(nextTask.args.size);
		for (sf::StringBuf &arg : nextTask.args) args.push(arg);

		sf::StringBuf commandLine;
		sf::Process *process = sf::startProcess(nextTask.exeName, args, nextTask.startOpts, &commandLine);
		if (process) {
			sf::debugPrintLine("> %s", commandLine.data);
			state.activeConcurrentThreads += nextTask.numThreads;

			ActiveProcess &active = state.activeProcesses.push();
			active.process = process;
			active.task = std::move(nextTask);
			active.commandLine = std::move(commandLine);
		}
	}
}
