#ifndef MX_PLATFORM_WIN32_INCLUDED_H
#define MX_PLATFORM_WIN32_INCLUDED_H

#include <intrin.h>
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// -- Language

#define mx_forceinline __forceinline
#define mx_noinline __declspec(noinline)
#define mx_unreachable() __debugbreak()
#define mx_assert(cond) do { if (!(cond)) __debugbreak(); } while (0)

// -- mxa_*

#if defined(_M_X64) || defined(_M_IX86)

// -- mxa_*32

#define mxa_load32(src) (*(const volatile uint32_t*)(src))
#define mxa_or32(dst, val) ((uint32_t)_InterlockedOr((volatile long*)(dst), (long)(val)))
#define mxa_and32(dst, val) ((uint32_t)_InterlockedAnd((volatile long*)(dst), (long)(val)))
#define mxa_inc32(dst) ((uint32_t)_InterlockedIncrement((volatile long*)(dst)) - 1)
#define mxa_dec32(dst) ((uint32_t)_InterlockedDecrement((volatile long*)(dst)) + 1)
#define mxa_add32(dst, val) ((uint32_t)_InterlockedExchangeAdd((volatile long*)(dst), (long)(val)))
#define mxa_sub32(dst, val) ((uint32_t)_InterlockedExchangeAdd((volatile long*)(dst), -(long)(val)))
#define mxa_cas32(dst, cmp, val) ((uint32_t)_InterlockedCompareExchange((volatile long*)(dst), (long)(val), (long)(cmp)) == (cmp))

#define mxa_load32_acq(src) mxa_load32(src)
#define mxa_or32_acq(dst, val) mxa_or32(dst, val)
#define mxa_and32_acq(dst, val) mxa_and32(dst, val)
#define mxa_inc32_acq(dst) mxa_inc32(dst)
#define mxa_dec32_acq(dst) mxa_dec32(dst)
#define mxa_add32_acq(dst, val) mxa_add32(dst, val)
#define mxa_sub32_acq(dst, val) mxa_sub32(dst, val)
#define mxa_cas32_acq(dst, cmp, val) mxa_cas32(dst, cmp, val)

#define mxa_load32_rel(src) mxa_load32(src)
#define mxa_or32_rel(dst, val) mxa_or32(dst, val)
#define mxa_and32_rel(dst, val) mxa_and32(dst, val)
#define mxa_inc32_rel(dst) mxa_inc32(dst)
#define mxa_dec32_rel(dst) mxa_dec32(dst)
#define mxa_add32_rel(dst, val) mxa_add32(dst, val)
#define mxa_sub32_rel(dst, val) mxa_sub32(dst, val)
#define mxa_cas32_rel(dst, cmp, val) mxa_cas32(dst, cmp, val)

#define mxa_load32_nf(src) mxa_load32(src)
#define mxa_or32_nf(dst, val) mxa_or32(dst, val)
#define mxa_and32_nf(dst, val) mxa_and32(dst, val)
#define mxa_inc32_nf(dst) mxa_inc32(dst)
#define mxa_dec32_nf(dst) mxa_dec32(dst)
#define mxa_add32_nf(dst, val) mxa_add32(dst, val)
#define mxa_sub32_nf(dst, val) mxa_sub32(dst, val)
#define mxa_cas32_nf(dst, cmp, val) mxa_cas32(dst, cmp, val)

// -- mxa_*_ptr

#define mxa_load_ptr(src) (*(void *const volatile*)(src))
#define mxa_cas_ptr(dst, cmp, val) (_InterlockedCompareExchangePointer((void*volatile*)(dst), (void*)(val), (void*)(cmp)) == (void*)(cmp))
#define mxa_exchange_ptr(dst, val) (_InterlockedExchangePointer((void*volatile*)(dst), (void*)(val)))

#define mxa_load_ptr_acq(src) mxa_load_ptr(src)
#define mxa_cas_ptr_acq(dst, cmp, val) mxa_cas_ptr(dst, cmp, val)
#define mxa_exchange_ptr_acq(dst, val) mxa_exchange_ptr(dst, val)

#define mxa_load_ptr_rel(src) mxa_load_ptr(src)
#define mxa_cas_ptr_rel(dst, cmp, val) mxa_cas_ptr(dst, cmp, val)
#define mxa_exchange_ptr_rel(dst, val) mxa_exchange_ptr(dst, val)

#define mxa_load_ptr_nf(src) mxa_load_ptr(src)
#define mxa_cas_ptr_nf(dst, cmp, val) mxa_cas_ptr(dst, cmp, val)
#define mxa_exchange_ptr_nf(dst, val) mxa_exchange_ptr(dst, val)

#elif defined(_M_ARM) || defined(_M_ARM64)

#define mxa_imp_mb() __dmb(0xB)

#define mxa_imp_load32(src) ((uint32_t)__iso_volatile_load32((const volatile __int32*)(src)))
#if defined(_M_ARM64)
	#define mxa_imp_load_ptr(src) ((void*)__iso_volatile_load64((const volatile __int64*)(src)))
#else
	#define mxa_imp_load_ptr(src) ((void*)__iso_volatile_load32((const volatile __int32*)(src)))
#endif

static mx_forceinline uint32_t mxa_load32(const void *src) { uint32_t v; mxa_imp_mb(); v = mxa_imp_load32(src); mxa_imp_mb(); return v; }
#define mxa_or32(dst, val) ((uint32_t)_InterlockedOr((volatile long*)(dst), (long)(val)))
#define mxa_and32(dst, val) ((uint32_t)_InterlockedAnd((volatile long*)(dst), (long)(val)))
#define mxa_inc32(dst) ((uint32_t)_InterlockedIncrement((volatile long*)(dst)) - 1)
#define mxa_dec32(dst) ((uint32_t)_InterlockedDecrement((volatile long*)(dst)) + 1)
#define mxa_add32(dst, val) ((uint32_t)_InterlockedExchangeAdd((volatile long*)(dst), (long)(val)))
#define mxa_sub32(dst, val) ((uint32_t)_InterlockedExchangeAdd((volatile long*)(dst)), -(long)(val))
#define mxa_cas32(dst, cmp, val) ((uint32_t)_InterlockedCompareExchange((volatile long*)(dst), (long)(val), (long)(cmp)) == (cmp))

static mx_forceinline uint32_t mxa_load32(const void *src) { uint32_t v; v = mxa_imp_load32(src); mxa_imp_mb(); return v; }
#define mxa_or32_acq(dst, val) ((uint32_t)_InterlockedOr_acq((volatile long*)(dst), (long)(val)))
#define mxa_and32_acq(dst, val) ((uint32_t)_InterlockedAnd_acq((volatile long*)(dst), (long)(val)))
#define mxa_inc32_acq(dst) ((uint32_t)_InterlockedIncrement_acq((volatile long*)(dst)) - 1)
#define mxa_dec32_acq(dst) ((uint32_t)_InterlockedDecrement_acq((volatile long*)(dst)) + 1)
#define mxa_add32_acq(dst, val) ((uint32_t)_InterlockedExchangeAdd_acq((volatile long*)(dst), (long)(val)))
#define mxa_sub32_acq(dst, val) ((uint32_t)_InterlockedExchangeAdd_acq((volatile long*)(dst)), -(long)(val))
#define mxa_cas32_acq(dst, cmp, val) ((uint32_t)_InterlockedCompareExchange_acq((volatile long*)(dst), (long)(val), (long)(cmp)) == (cmp))

static mx_forceinline uint32_t mxa_load32(const void *src) { uint32_t v; mxa_imp_mb(); v = mxa_imp_load32(src); return v; }
#define mxa_or32_rel(dst, val) ((uint32_t)_InterlockedOr_rel((volatile long*)(dst), (long)(val)))
#define mxa_and32_rel(dst, val) ((uint32_t)_InterlockedAnd_rel((volatile long*)(dst), (long)(val)))
#define mxa_inc32_rel(dst) ((uint32_t)_InterlockedIncrement_rel((volatile long*)(dst)) - 1)
#define mxa_dec32_rel(dst) ((uint32_t)_InterlockedDecrement_rel((volatile long*)(dst)) + 1)
#define mxa_add32_rel(dst, val) ((uint32_t)_InterlockedExchangeAdd_rel((volatile long*)(dst), (long)(val)))
#define mxa_sub32_rel(dst, val) ((uint32_t)_InterlockedExchangeAdd_rel((volatile long*)(dst)), -(long)(val))
#define mxa_cas32_rel(dst, cmp, val) ((uint32_t)_InterlockedCompareExchange_rel((volatile long*)(dst), (long)(val), (long)(cmp)) == (cmp))

#define mxa_load32_nf(src) mxa_imp_load32(src)
#define mxa_or32_nf(dst, val) ((uint32_t)_InterlockedOr_nf((volatile long*)(dst), (long)(val)))
#define mxa_and32_nf(dst, val) ((uint32_t)_InterlockedAnd_nf((volatile long*)(dst), (long)(val)))
#define mxa_inc32_nf(dst) ((uint32_t)_InterlockedIncrement_nf((volatile long*)(dst)) - 1)
#define mxa_dec32_nf(dst) ((uint32_t)_InterlockedDecrement_nf((volatile long*)(dst)) + 1)
#define mxa_add32_nf(dst, val) ((uint32_t)_InterlockedExchangeAdd_nf((volatile long*)(dst), (long)(val)))
#define mxa_sub32_nf(dst, val) ((uint32_t)_InterlockedExchangeAdd_nf((volatile long*)(dst)), -(long)(val))
#define mxa_cas32_nf(dst, cmp, val) ((uint32_t)_InterlockedCompareExchange_nf((volatile long*)(dst), (long)(val), (long)(cmp)) == (cmp))

static mx_forceinline void *mxa_load_ptr(const void *src) { void *v; mxa_imp_mb(); v = mxa_imp_load_ptr(src); mxa_imp_mb(); return v; }
#define mxa_cas_ptr(dst, cmp, val) (_InterlockedCompareExchangePointer((void*volatile*)(dst), (void*)(val), (void*)(cmp)) == (void*)(cmp))

static mx_forceinline void *mxa_load_ptr_acq(const void *src) { void *v; v = mxa_imp_load_ptr(src); mxa_imp_mb(); return v; }
#define mxa_cas_ptr_acq(dst, cmp, val) (_InterlockedCompareExchangePointer_acq((void*volatile*)(dst), (void*)(val), (void*)(cmp)) == (void*)(cmp))

static mx_forceinline void *mxa_load_ptr_rel(const void *src) { void *v; mxa_imp_mb(); v = mxa_imp_load_ptr(src); return v; }
#define mxa_cas_ptr_rel(dst, cmp, val) (_InterlockedCompareExchangePointer_rel((void*volatile*)(dst), (void*)(val), (void*)(cmp)) == (void*)(cmp))

#define mxa_load_ptr_nf(src) mxa_imp_load_ptr(src)
#define mxa_cas_ptr_nf(dst, cmp, val) (_InterlockedCompareExchangePointer_nf((void*volatile*)(dst), (void*)(val), (void*)(cmp)) == (void*)(cmp))

#else
	#error "Unsupported architecture"
#endif

// -- Misc intrinsics

static mx_forceinline uint32_t mx_ctz32(uint32_t mask)
{
	unsigned long index;
	_BitScanForward(&index, (unsigned long)mask);
	return index;
}

// -- Threads

#if defined(_M_X64)
	#define mx_get_thread_id() ((uint32_t)__readgsdword(0x48))
#elif defined(_M_IX86)
	#define mx_get_thread_id() ((uint32_t)__readfsdword(0x24))
#elif defined(_M_ARM)
	#define mx_get_thread_id() (((const uint32_t*)NtCurrentTeb())[9])
#else
	#error "Unsupported architecture"
#endif

#define mx_yield() _mm_pause()

// -- mx_os_semaphore

typedef HANDLE mx_os_semaphore;

static mx_forceinline void mx_os_semaphore_init(mx_os_semaphore *s)
{
	*s = CreateSemaphoreA(NULL, 0, (LONG)INT32_MAX, NULL);
}

static mx_forceinline void mx_os_semaphore_free(mx_os_semaphore *s)
{
	CloseHandle(*s);
	*s = NULL;
}

static mx_forceinline void mx_os_semaphore_wait(mx_os_semaphore *s)
{
	WaitForSingleObject(*s, INFINITE);
}

static mx_forceinline void mx_os_semaphore_signal(mx_os_semaphore *s)
{
	ReleaseSemaphore(*s, 1, NULL);
}

static mx_forceinline void mx_os_semaphore_wait_n(mx_os_semaphore *s, uint32_t count)
{
	do {
		WaitForSingleObject(*s, INFINITE);
	} while (--count > 0);
}

static mx_forceinline void mx_os_semaphore_signal_n(mx_os_semaphore *s, uint32_t count)
{
	ReleaseSemaphore(*s, (LONG)count, NULL);
}

#endif
