#include "Process.h"
#include "sf/Internal.h"

#if SF_OS_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

namespace sf {

#if SF_OS_WINDOWS

struct Process
{
	PROCESS_INFORMATION info;
	HANDLE hStdOut = NULL;
	HANDLE hStdErr = NULL;
	HANDLE hStdIn = NULL;
};

static bool needsEscape(sf::Slice<wchar_t> argW)
{
	if (argW.size == 0) return true;
	for (wchar_t w : argW) {
		if (w == ' ' || w == '\t' || w == '\n' || w == '\v' || w == '\t' || w == '"') return true;
	}
	return false;
}

static void pushBackslashes(sf::Array<wchar_t> &commandLineW, size_t num)
{
	for (size_t i = 0; i < num; i++) {
		commandLineW.push('\\');
	}
}

static bool escapeArgs(sf::Array<wchar_t> &commandLineW, sf::Slice<sf::String> args)
{
	if (args.size && commandLineW.size) commandLineW.pop();

	sf::SmallArray<wchar_t, 256> argW;
	for (sf::String &arg : args) {
		bool lastArg = &arg + 1 == args.end();

		commandLineW.push(' ');

		argW.clear();
		if (!sf::win32Utf8To16(argW, arg)) return false;
		argW.pop();

		if (needsEscape(argW)) {
			commandLineW.push('"');

			for (size_t i = 0; i < argW.size; i++) {
				size_t numBs = 0;
				for (; i < argW.size && argW[i] == '\\'; i++, numBs++) { }
				if (i == argW.size) {
					// Need to escape remaining backslashes before the closing
					// quote on the last argument
					pushBackslashes(commandLineW, lastArg ? numBs * 2 : numBs);
				} else if (argW[i] == '"') {
					// Need to escape the backslashes before the quote
					pushBackslashes(commandLineW, numBs * 2);
					commandLineW.push(L"\\\"", 2);
				} else {
					// Push backslashes un-escaped
					pushBackslashes(commandLineW, numBs);
					commandLineW.push(argW[i]);
				}
			}

			commandLineW.push('"');
		} else {
			commandLineW.push(argW);
		}
	}
	commandLineW.push('\0');

	return true;
}

Process *startProcess(sf::String path, sf::Slice<sf::String> args, const ProcessStartOpts &opts, sf::StringBuf *pCommandLine)
{
	sf::SmallArray<wchar_t, 1024> commandLineW;
	if (!sf::win32Utf8To16(commandLineW, path)) return NULL;
	if (!escapeArgs(commandLineW, args)) return NULL;

	if (pCommandLine) {
		sf::win32Utf16To8(*pCommandLine, commandLineW.data);
	}

	Process *p = new Process();

	STARTUPINFOW startInfo = { };
	startInfo.cb = sizeof(STARTUPINFOW);

	BOOL inheritHandles = FALSE;

	if (opts.attachStdIo) {
		startInfo.dwFlags |= STARTF_USESTDHANDLES;
		inheritHandles = TRUE;

		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa); 
		sa.bInheritHandle = TRUE; 
		sa.lpSecurityDescriptor = NULL; 

		CreatePipe(&p->hStdOut, &startInfo.hStdOutput, &sa, 0);
		CreatePipe(&p->hStdErr, &startInfo.hStdError, &sa, 0);
		CreatePipe(&startInfo.hStdInput, &p->hStdIn, &sa, 0);
		SetHandleInformation(p->hStdOut, HANDLE_FLAG_INHERIT, 0);
		SetHandleInformation(p->hStdErr, HANDLE_FLAG_INHERIT, 0);
	}

	DWORD flags = CREATE_NO_WINDOW;

	if (opts.affinityMask) flags |= CREATE_SUSPENDED;

	if (!CreateProcessW(NULL, commandLineW.data, NULL, NULL, inheritHandles, flags, NULL, NULL, &startInfo, &p->info)) {
		delete p;
		return NULL;
	}

	if (opts.affinityMask) {
		SetProcessAffinityMask(p->info.hProcess, opts.affinityMask);
		ResumeThread(p->info.hThread);
	}

	if (opts.attachStdIo) {
		CloseHandle(startInfo.hStdOutput);
		CloseHandle(startInfo.hStdError);
		CloseHandle(startInfo.hStdInput);
	}

	return p;
}

bool getProcessCompleted(Process *p, uint32_t *pCode)
{
	uint32_t dummy;
	if (!pCode) pCode = &dummy;
	if (!GetExitCodeProcess(p->info.hProcess, (LPDWORD)pCode)) return false;
	return *pCode != STILL_ACTIVE;
}

void joinProcess(Process *p)
{
	WaitForSingleObject(p->info.hProcess, INFINITE);
	detachProcess(p);
}

void detachProcess(Process *p)
{
	CloseHandle(p->info.hProcess);
	CloseHandle(p->info.hThread);
	if (p->hStdOut) CloseHandle(p->hStdOut);
	if (p->hStdErr) CloseHandle(p->hStdErr);
	if (p->hStdIn) CloseHandle(p->hStdIn);
	delete p;
}

size_t readProcessStdOut(Process *p, void *dst, size_t size)
{
	DWORD num = 0;
	if (!ReadFile(p->hStdOut, dst, (DWORD)size, &num, NULL)) return 0;
	return num;
}

size_t readProcessStdErr(Process *p, void *dst, size_t size)
{
	DWORD num = 0;
	if (!ReadFile(p->hStdErr, dst, (DWORD)size, &num, NULL)) return 0;
	return num;
}

#else

Process *startProcess(sf::String path, sf::Slice<sf::String> args, const ProcessStartOpts &opts, sf::StringBuf *pCommandLine)
{
    return NULL;
}

bool getProcessCompleted(Process *p, uint32_t *pCode) { return false; }
void joinProcess(Process *p) { }
void detachProcess(Process *p) { }
size_t readProcessStdOut(Process *p, void *dst, size_t size) { return 0; }
size_t readProcessStdErr(Process *p, void *dst, size_t size) { return 0; }

#endif

}
