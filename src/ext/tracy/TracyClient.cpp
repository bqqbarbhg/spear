//
//          Tracy profiler
//         ----------------
//
// For fast integration, compile and
// link with this source file (and none
// other) in your executable (or in the
// main DLL / shared object on multi-DLL
// projects).
//

// Define TRACY_ENABLE to enable profiler.

#include "common/TracySystem.cpp.h"

#ifdef TRACY_ENABLE

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif

#include "common/tracy_lz4.cpp.h"
#include "client/TracyProfiler.cpp.h"
#include "client/TracyCallstack.cpp.h"
#include "client/TracySysTime.cpp.h"
#include "client/TracySysTrace.cpp.h"
#include "common/TracySocket.cpp.h"
#include "client/tracy_rpmalloc.cpp.h"
#include "client/TracyDxt1.cpp.h"

#ifdef _MSC_VER
#  pragma comment(lib, "ws2_32.lib")
#  pragma comment(lib, "dbghelp.lib")
#  pragma warning(pop)
#endif

#endif
