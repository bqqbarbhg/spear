#include "Thread.h"
#include "Internal.h"

#if SF_OS_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#elif SF_USE_PTHREADS
    #include <pthread.h>

	#if SF_OS_EMSCRIPTEN
		#include <emscripten.h>
		#include <emscripten/threading.h>
    #else
        #include <time.h>
	#endif

#endif

#if SF_OS_WINDOWS

// Cursed code from MSDN:
// https://docs.microsoft.com/en-us/visualstudio/debugger/how-to-set-a-thread-name-in-native-code

// Usage: SetThreadName ((DWORD)-1, "MainThread");
const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)
void SetThreadName(DWORD dwThreadID, const char* threadName) {
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable: 6320 6322)
    __try{
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER){
    }
#pragma warning(pop)
}

#endif

namespace sf {

#if SF_OS_WINDOWS

void setDebugThreadName(sf::String name)
{
    sf::StringBuf nameBuf(name);

    typedef HRESULT (WINAPI *pfn_SetThreadDescription)(HANDLE, PCWSTR);
    static pfn_SetThreadDescription setThreadDescription = (pfn_SetThreadDescription)GetProcAddress(LoadLibraryA("Kernel32.dll"), "SetThreadDescription");

    SetThreadName(GetCurrentThreadId(), nameBuf.data);
    if (setThreadDescription) {
        Array<wchar_t> wideName;
        win32Utf8To16(wideName, name);
		setThreadDescription(GetCurrentThread(), wideName.data);
    }
}

struct ThreadImp : Thread
{
    ThreadEntry entry;
    void *user;
    sf::StringBuf debugName;
    HANDLE handle;
    DWORD id;
};

DWORD WINAPI threadEntry(LPVOID arg)
{
    ThreadImp *imp = (ThreadImp*)arg;
    if (imp->debugName.size > 0) {
        setDebugThreadName(imp->debugName);
    }

    imp->entry(imp->user);
    return 0;
}

Thread *Thread::start(const ThreadDesc &desc)
{
    ThreadImp *imp = new ThreadImp();
    if (!imp) return nullptr;

    imp->debugName = desc.name; 
    imp->entry = desc.entry;
    imp->user = desc.user;
    imp->handle = CreateThread(NULL, 0, &threadEntry, imp, 0, &imp->id);

    return imp;
}

void Thread::join(Thread *thread)
{
    if (!thread) return;
    ThreadImp *imp = (ThreadImp*)thread;
    WaitForSingleObject(imp->handle, INFINITE);
    CloseHandle(imp->handle);
    delete imp;
}

void Thread::sleepMs(uint32_t ms)
{
    Sleep(ms);
}

#elif SF_USE_PTHREADS

void setDebugThreadName(sf::String name)
{
    sf::StringBuf nameBuf(name);

#if SF_OS_EMSCRIPTEN
    emscripten_set_thread_name(pthread_self(), nameBuf.data);
#endif
}

struct ThreadImp : Thread
{
    ThreadEntry entry;
    void *user;
    sf::StringBuf debugName;
    pthread_t thread;
};

void *threadEntry(void *arg)
{
    ThreadImp *imp = (ThreadImp*)arg;
    if (imp->debugName.size > 0) {
        setDebugThreadName(imp->debugName);
    }
    imp->entry(imp->user);
    return 0;
}

Thread *Thread::start(const ThreadDesc &desc)
{
    ThreadImp *imp = new ThreadImp();
    if (!imp) return nullptr;

    imp->debugName = desc.name; 
    imp->entry = desc.entry;
    imp->user = desc.user;
    pthread_create(&imp->thread, NULL, &threadEntry, imp);

    return imp;
}

void Thread::join(Thread *thread)
{
    if (!thread) return;
    ThreadImp *imp = (ThreadImp*)thread;
    pthread_join(imp->thread, nullptr);
    delete imp;
}

void Thread::sleepMs(uint32_t ms)
{
#if SF_OS_EMSCRIPTEN
    emscripten_sleep(ms);
#else
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    while (nanosleep(&ts, &ts)) { }
#endif
}

#else

void setDebugThreadName(sf::String name)
{
}

Thread *Thread::start(const ThreadDesc &desc)
{
    return nullptr;
}

void Thread::join(Thread *thread)
{
}

void Thread::sleepMs(uint32_t ms)
{
}

#endif

}
