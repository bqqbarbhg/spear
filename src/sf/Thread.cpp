#include "Thread.h"
#include "Internal.h"

#if SF_OS_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
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

void setDebugThreadName(sf::String name)
{
    sf::StringBuf nameBuf(name);

#if SF_OS_WINDOWS
    typedef HRESULT (*pfn_SetThreadDescription)(HANDLE, PCWSTR);
    static pfn_SetThreadDescription setThreadDescription = (pfn_SetThreadDescription)GetProcAddress(LoadLibraryA("Kernel32.dll"), "SetThreadDescription");

    SetThreadName(GetCurrentThreadId(), nameBuf.data);
    if (setThreadDescription) {
        Array<wchar_t> wideName;
        win32Utf8To16(wideName, name);
		setThreadDescription(GetCurrentThread(), wideName.data);
    }
#endif
}

}
