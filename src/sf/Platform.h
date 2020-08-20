#pragma once

// Base platform header, must compile as both C/C++ and be more portable
// than all the other headers, as it's used for integrating the framework
// to other libraries (which may be written in C)

#ifndef SF_PLATFORM_H
#define SF_PLATFORM_H

// -- Compiler detection

#define SF_CC_MSC 0     // < Microsoft Visual C++ compiler
#define SF_CC_GNU 0     // < GNU-compatible compler, GCC or Clang
#define SF_CC_CLANG 0   // < Clang only (SF_CC_GNU is also defined)
#define SF_CC_GENERIC 0 // < Unknown / generic compiler

#if defined(SF_DEF_CC_GENERIC) || defined(SF_DEF_GENERIC)
	#define SF_FOUND_CC 1
#endif

#if !defined(SF_FOUND_CC) && defined(_MSC_VER)
	#undef SF_CC_MSC
	#define SF_CC_MSC 1
	#define SF_FOUND_CC 1
#endif

#if !defined(SF_FOUND_CC) && defined(__clang__)
	#undef SF_CC_GNU
	#undef SF_CC_CLANG
	#define SF_CC_GNU 1
	#define SF_CC_CLANG 1
	#define SF_FOUND_CC 1
#endif

#if !defined(SF_FOUND_CC) && defined(__GNUC__)
	#undef SF_CC_GNU
	#define SF_CC_GNU 1
	#define SF_FOUND_CC 1
#endif

#if !defined(SF_FOUND_CC) || (defined(SF_DEF_CC_GENERIC) || defined(SF_DEF_GENERIC))
	#undef SF_CC_GENERIC 1
	#define SF_CC_GENERIC 1
	#define SF_FOUND_CC 1
#endif

// -- OS detection

#define SF_OS_WINDOWS 0    // < Win32 desktop application
#define SF_OS_LINUX 0      // < Linux desktop application
#define SF_OS_WASM 0       // < WebAssembly (but maybe not Emscripten)
#define SF_OS_EMSCRIPTEN 0 // < Emscripten (with SF_OS_WASM)
#define SF_OS_APPLE 0      // < Any Apple OS (MacOS or iOS)
#define SF_OS_GENERIC 0    // < Modern C++11-17 OS wrappers

#if defined(SF_DEF_OS_WIDOWS) || defined(SF_DEF_OS_LINUX) || defined(SF_DEF_OS_WASM) || defined(SF_DEF_OS_EMSCRIPTEN) || defined(SF_DEF_OS_APPLE) || defined(SF_DEF_OS_GENERIC) || defined(SF_DEF_GENERIC)
	#define SF_FOUND_OS 1
#endif

#if !defined(SF_FOUND_OS) && defined(_WIN32) || defined(SF_DEF_OS_WINDOWS)
	#undef SF_OS_WINDOWS
	#define SF_OS_WINDOWS 1
	#define SF_FOUND_OS 1
#endif

#if !defined(SF_FOUND_OS) && defined(__linux__) || defined(SF_DEF_OS_LINUX)
	#undef SF_OS_LINUX
	#define SF_OS_LINUX 1
	#define SF_FOUND_OS 1
#endif

#if !defined(SF_FOUND_OS) && defined(__EMSCRIPTEN__) || defined(SF_DEF_OS_EMSCRIPTEN)
	#undef SF_OS_WASM
	#undef SF_OS_EMSCRIPTEN
	#define SF_OS_WASM 1
	#define SF_OS_EMSCRIPTEN 1
	#define SF_FOUND_OS 1
#endif

#if !defined(SF_FOUND_OS) && defined(__wasm__) || defined(SF_DEF_OS_WASM)
	#undef SF_OS_WASM
	#define SF_OS_WASM 1
	#define SF_FOUND_OS 1
#endif

#if !defined(SF_FOUND_OS) && defined(__APPLE__) || defined(SF_DEF_OS_APPLE)
    #undef SF_OS_APPLE
    #define SF_OS_APPLE 1
    #define SF_FOUND_OS 1
#endif

#if !defined(SF_FOUND_OS) || (defined(SF_DEF_OS_GENERIC) || defined(SF_DEF_GENERIC))
	#undef SF_OS_GENERIC
	#define SF_OS_GENERIC 1
	#define SF_FOUND_OS 1
#endif

// -- Architecture detection

#define SF_ARCH_X86 0     // < 32-bit or 64-bit x86
#define SF_ARCH_X64 0     // < 64-bit x86 (SF_ARCH_X86 is also defined)
#define SF_ARCH_ARM 0     // < 32-bit or 64-bit ARM
#define SF_ARCH_ARM64 0   // < 64-bit ARM (SF_ARCH_ARM is also defined)
#define SF_ARCH_WASM 0    // < WebAssembly
#define SF_ARCH_GENERIC 0 // < C++ virtual machine

#if defined(SF_DEF_ARCH_GENERIC) || defined(SF_DEF_GENERIC)
	#define SF_FOUND_ARCH 1
#endif

#if !defined(SF_FOUND_ARCH) && ((SF_CC_MSC && defined(_M_X64)) || (SF_CC_GNU && defined(__x86_64__)))
	#undef SF_ARCH_X86
	#undef SF_ARCH_X64
	#define SF_ARCH_X86 1
	#define SF_ARCH_X64 1
	#define SF_FOUND_ARCH 1
#endif

#if !defined(SF_FOUND_ARCH) && ((SF_CC_MSC && defined(_M_IX86)) || (SF_CC_GNU && defined(__i386__)))
	#undef SF_ARCH_X86
	#define SF_ARCH_X86 1
	#define SF_FOUND_ARCH 1
#endif

#if !defined(SF_FOUND_ARCH) && ((SF_CC_MSC && defined(_M_ARM64)) || (SF_CC_GNU && defined(__aarch64__)))
	#undef SF_ARCH_ARM
	#undef SF_ARCH_ARM64
	#define SF_ARCH_ARM 1
	#define SF_ARCH_ARM64 1
	#define SF_FOUND_ARCH 1
#endif

#if !defined(SF_FOUND_ARCH) && ((SF_CC_MSC && defined(_M_ARM)) || (SF_CC_GNU && defined(__arm__)))
	#undef SF_ARCH_ARM
	#define SF_ARCH_ARM 1
	#define SF_FOUND_ARCH 1
#endif

#if !defined(SF_FOUND_ARCH) && (SF_CC_GNU && defined(__wasm__))
	#undef SF_ARCH_WASM
	#define SF_ARCH_WASM 1
	#define SF_FOUND_ARCH 1
#endif

#if !defined(SF_FOUND_ARCH) || (defined(SF_DEF_ARCH_GENERIC) || defined(SF_DEF_GENERIC))
	#undef SF_ARCH_GENERIC
	#define SF_ARCH_GENERIC 1
	#define SF_FOUND_ARCH 1
#endif

// -- Build type

#if (SF_CC_MSC && defined(_DEBUG)) || (!SF_CC_MSC && !defined(NDEBUG))
	#define SF_DEBUG 1
#else
	#define SF_DEBUG 0
#endif

// -- Language extensions

#if SF_CC_MSC
	#define sf_forceinline __forceinline inline
	#define sf_noinline __declspec(noinline)
	#define sf_malloc_like __declspec(restrict)
#elif SF_CC_GNU
	#define sf_forceinline __attribute__((always_inline)) inline
	#define sf_noinline __attribute__((noinline))
	#define sf_malloc_like __attribute__((malloc))
#else
	#define sf_forceinline
	#define sf_noinline
	#define sf_malloc_like
#endif

#define sf_inline static sf_forceinline

#define sf_arraysize(arr) (sizeof(arr) / sizeof(*(arr)))

// -- Platform

#if SF_ARCH_X64 || SF_ARCH_ARM64
	#define SF_FAST_64BIT 1
#elif SF_ARCH_WASM
	#if defined(SF_DEF_WASM_64BIT)
		#define SF_FAST_64BIT 1
	#else
		#define SF_FAST_64BIT 0
	#endif
#elif SF_ARCH_X86 || SF_ARCH_ARM || SF_ARCH_GENERIC
	#define SF_FAST_64BIT 0
#endif

#if SF_ARCH_WASM && defined(__EMSCRIPTEN_PTHREADS__)
	#define SF_USE_PTHREADS 1
#else
	#define SF_USE_PTHREADS 0
#endif

#if !defined(SF_WASM_USE_SIMD)
	#define SF_WASM_USE_SIMD 0
#endif

#if SF_ARCH_X64 || SF_ARCH_ARM64
	#define SF_POINTER_64BIT 1
#else
	#define SF_POINTER_64BIT 0
#endif

// -- Debugging

#if SF_CC_MSC
	#define sf_debugbreak() __debugbreak()
#elif SF_OS_WASM
	#define sf_debugbreak() sf_wasm_debugbreak(__FILE__, __LINE__)
#elif SF_CC_GNU
	#define sf_debugbreak() __builtin_trap()
#else
	#include <stdlib.h>
	#define sf_debugbreak() abort()
#endif

#if SF_DEBUG
	#define sf_assert(cond) do { if (!(cond)) sf_debugbreak(); } while (0)
	#define sf_fail() sf_debugbreak()
#else
	#define sf_assert(cond) (void)0
	#define sf_fail() (void)0
#endif

#if SF_CC_MSC
	#pragma warning(disable: 4141) // warning C4141: 'inline': used more than once
#endif

#ifdef __cplusplus
extern "C" {
#endif

void sf_debug_log(const char *str);
void sf_debug_logf(const char *fmt, ...);

#if SF_OS_WASM
void sf_wasm_debugbreak(const char *file, int line);
#endif

void sf_set_debug_thread_name(const char *name);

#ifdef __cplusplus
}
#endif

#endif

#ifndef SF_USE_MIMALLOC
#define SF_USE_MIMALLOC 0
#endif

#if SF_USE_MIMALLOC
	#include "ext/mimalloc/mimalloc.h"
	#define sf_malloc(size) mi_malloc((size))
	#define sf_realloc(ptr, size) mi_realloc((ptr), (size))
	#define sf_calloc(num, size) mi_calloc((num), (size))
	#define sf_free(ptr) mi_free((ptr))

	#define sf_malloc_aligned(size, align) mi_malloc_aligned((size), (align))
	#define sf_free_aligned(ptr, align) ((void)(align), mi_free((ptr)))
#else
	#include <stdlib.h>
	#define sf_malloc(size) malloc((size))
	#define sf_calloc(num, size) calloc((num), (size))
	#define sf_realloc(ptr, size) realloc((ptr), (size))
	#define sf_free(ptr) free((ptr))

	#if SF_CC_MSC
		#include <malloc.h>
		#define sf_malloc_aligned(size, align) _aligned_malloc((size), (align))
		#define sf_free_aligned(ptr, align) ((void)align, _aligned_free((ptr)))
	#else
		#define sf_malloc_aligned(size, align) aligned_alloc((align), (size))
		#define sf_free_aligned(ptr, align) ((void)align, free((ptr)))
	#endif
#endif
