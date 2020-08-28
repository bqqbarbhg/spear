#ifndef MX_PLATFORM_POSIX_INCLUDED_H
#define MX_PLATFORM_POSIX_INCLUDED_H

#define MX_PLATFORM_POSIX

#include <errno.h>
#include <semaphore.h>
#include <stdint.h>
#include <string.h>

// -- Language

#define mx_forceinline inline __attribute__((always_inline))
#define mx_noinline __attribute__((noinline))
#define mx_unreachable() __builtin_trap()
#define mx_assert(cond) do { if (!(cond)) __builtin_trap(); } while (0)

// -- mxa_*

// -- mxa_*32

#define mxa_load32(src) __c11_atomic_load((_Atomic(uint32_t) *)(src), __ATOMIC_SEQ_CST)
#define mxa_or32(dst, val) __c11_atomic_fetch_or((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_SEQ_CST)
#define mxa_and32(dst, val) __c11_atomic_fetch_and((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_SEQ_CST)
#define mxa_inc32(dst) __c11_atomic_fetch_add((_Atomic(uint32_t) *)(dst), 1u, __ATOMIC_SEQ_CST)
#define mxa_dec32(dst) __c11_atomic_fetch_sub((_Atomic(uint32_t) *)(dst), 1u, __ATOMIC_SEQ_CST)
#define mxa_add32(dst, val) __c11_atomic_fetch_add((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_SEQ_CST)
#define mxa_sub32(dst, val) __c11_atomic_fetch_sub((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_SEQ_CST)
static mx_forceinline int mxa_cas32(void *dst, uint32_t cmp, uint32_t val) { return __c11_atomic_compare_exchange_strong((_Atomic(uint32_t) *)dst, &cmp, val, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); }

#define mxa_load32_acq(src) __c11_atomic_load((_Atomic(uint32_t) *)(src), __ATOMIC_ACQUIRE)
#define mxa_or32_acq(dst, val) __c11_atomic_fetch_or((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_ACQUIRE)
#define mxa_and32_acq(dst, val) __c11_atomic_fetch_and((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_ACQUIRE)
#define mxa_inc32_acq(dst) __c11_atomic_fetch_add((_Atomic(uint32_t) *)(dst), 1u, __ATOMIC_ACQUIRE)
#define mxa_dec32_acq(dst) __c11_atomic_fetch_sub((_Atomic(uint32_t) *)(dst), 1u, __ATOMIC_ACQUIRE)
#define mxa_add32_acq(dst, val) __c11_atomic_fetch_add((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_ACQUIRE)
#define mxa_sub32_acq(dst, val) __c11_atomic_fetch_sub((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_ACQUIRE)
static mx_forceinline int mxa_cas32_acq(void *dst, uint32_t cmp, uint32_t val) { return __c11_atomic_compare_exchange_strong((_Atomic(uint32_t) *)dst, &cmp, val, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); }

#define mxa_load32_rel(src) __c11_atomic_load((_Atomic uint32_t*)(src), __ATOMIC_RELEASE)
#define mxa_or32_rel(dst, val) __c11_atomic_fetch_or((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_RELEASE)
#define mxa_and32_rel(dst, val) __c11_atomic_fetch_and((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_RELEASE)
#define mxa_inc32_rel(dst) __c11_atomic_fetch_add((_Atomic(uint32_t) *)(dst), 1u, __ATOMIC_RELEASE)
#define mxa_dec32_rel(dst) __c11_atomic_fetch_sub((_Atomic(uint32_t) *)(dst), 1u, __ATOMIC_RELEASE)
#define mxa_add32_rel(dst, val) __c11_atomic_fetch_add((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_RELEASE)
#define mxa_sub32_rel(dst, val) __c11_atomic_fetch_sub((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_RELEASE)
static mx_forceinline int mxa_cas32_rel(void *dst, uint32_t cmp, uint32_t val) { return __c11_atomic_compare_exchange_strong((_Atomic(uint32_t) *)dst, &cmp, val, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); }

#define mxa_load32_nf(src) __c11_atomic_load((_Atomic(uint32_t) *)(src), __ATOMIC_RELAXED)
#define mxa_or32_nf(dst, val) __c11_atomic_fetch_or((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_RELAXED)
#define mxa_and32_nf(dst, val) __c11_atomic_fetch_and((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_RELAXED)
#define mxa_inc32_nf(dst) __c11_atomic_fetch_add((_Atomic(uint32_t) *)(dst), 1u, __ATOMIC_RELAXED)
#define mxa_dec32_nf(dst) __c11_atomic_fetch_sub((_Atomic(uint32_t) *)(dst), 1u, __ATOMIC_RELAXED)
#define mxa_add32_nf(dst, val) __c11_atomic_fetch_add((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_RELAXED)
#define mxa_sub32_nf(dst, val) __c11_atomic_fetch_sub((_Atomic(uint32_t) *)(dst), (val), __ATOMIC_RELAXED)
static mx_forceinline int mxa_cas32_nf(void *dst, uint32_t cmp, uint32_t val) { return __c11_atomic_compare_exchange_strong((_Atomic(uint32_t) *)dst, &cmp, val, __ATOMIC_RELAXED, __ATOMIC_RELAXED); }

// -- mxa_*_ptr

#define mxa_load_ptr(src) (void*)__c11_atomic_load((_Atomic(void*) *)(src), __ATOMIC_SEQ_CST)
static mx_forceinline int mxa_cas_ptr(void *dst, const void *cmp, const void *val) { return __c11_atomic_compare_exchange_strong((_Atomic(void*) *)dst, (void**)&cmp, (void*)val, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); }
#define mxa_exchange_ptr(src, dst) (void*)__c11_atomic_exchange((_Atomic(void*) *)(src), (void*)(dst), __ATOMIC_SEQ_CST)

#define mxa_load_ptr_acq(src) (void*)__c11_atomic_load((_Atomic(void*) *)(src), __ATOMIC_ACQUIRE)
static mx_forceinline int mxa_cas_ptr_acq(void *dst, const void *cmp, const void *val) { return __c11_atomic_compare_exchange_strong((_Atomic(void*) *)dst, (void**)&cmp, (void*)val, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); }
#define mxa_exchange_ptr_acq(src, dst) (void*)__c11_atomic_exchange((_Atomic(void*) *)(src), (void*)(dst), __ATOMIC_ACQUIRE)

#define mxa_load_ptr_rel(src) (void*)__c11_atomic_load((_Atomic(void*) *)(src), __ATOMIC_RELEASE)
static mx_forceinline int mxa_cas_ptr_rel(void *dst, const void *cmp, const void *val) { return __c11_atomic_compare_exchange_strong((_Atomic(void*) *)dst, (void**)&cmp, (void*)val, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST); }
#define mxa_exchange_ptr_rel(src, dst) (void*)__c11_atomic_exchange((_Atomic(void*) *)(src), (void*)(dst), __ATOMIC_RELEASE)

#define mxa_load_ptr_nf(src) (void*)__c11_atomic_load((_Atomic(void*) *)(src), __ATOMIC_RELAXED)
static mx_forceinline int mxa_cas_ptr_nf(void *dst, const void *cmp, const void *val) { return __c11_atomic_compare_exchange_strong((_Atomic(void*) *)dst, (void**)&cmp, (void*)val, __ATOMIC_RELAXED, __ATOMIC_RELAXED); }
#define mxa_exchange_ptr_nf(src, dst) (void*)__c11_atomic_exchange((_Atomic(void*) *)(src), (void*)(dst), __ATOMIC_RELAXED)

// -- Misc intrinsics

#define mx_ctz32(mask) (uint32_t)__builtin_ctz((unsigned int)(mask))

// -- Threads

// TODO: Maybe there's a better native way?
extern uint32_t mx_imp_next_thread_id;
extern __thread uint32_t mx_imp_thread_id;
static mx_forceinline uint32_t mx_get_thread_id()
{
	uint32_t id = mx_imp_thread_id;
	if (!id) {
		id = mxa_inc32(&mx_imp_next_thread_id) + 1;
		mx_imp_thread_id = id;
	}
	return id;
}

#define mx_yield()

// -- mx_os_semaphore

#if defined(__MACH__)

#include <dispatch/dispatch.h>

typedef dispatch_semaphore_t mx_os_semaphore;

static void mx_os_semaphore_init(mx_os_semaphore *s)
{
    *s = dispatch_semaphore_create(0);
}

static void mx_os_semaphore_free(mx_os_semaphore *s)
{
    dispatch_release(*s);
}

static void mx_os_semaphore_wait(mx_os_semaphore *s)
{
    dispatch_semaphore_wait(*s, DISPATCH_TIME_FOREVER);
}

static void mx_os_semaphore_signal(mx_os_semaphore *s)
{
    dispatch_semaphore_signal(*s);
}

static void mx_os_semaphore_wait_n(mx_os_semaphore *s, uint32_t count)
{
    do {
        dispatch_semaphore_wait(*s, DISPATCH_TIME_FOREVER);
    } while (--count > 0);
}

static void mx_os_semaphore_signal_n(mx_os_semaphore *s, uint32_t count)
{
    do {
        dispatch_semaphore_signal(*s);
    } while (--count > 0);
}

#else

typedef sem_t mx_os_semaphore;

static void mx_os_semaphore_init(mx_os_semaphore *s)
{
	sem_init(s, 0, 0);
}

static void mx_os_semaphore_free(mx_os_semaphore *s)
{
	sem_destroy(s);
	memset(s, 0, sizeof(mx_os_semaphore));
}

static void mx_os_semaphore_wait(mx_os_semaphore *s)
{
	while (sem_wait(s) != 0) {
		if (errno != EINTR) {
			__builtin_trap();
		}
	}
}

static void mx_os_semaphore_signal(mx_os_semaphore *s)
{
	sem_post(s);
}

static void mx_os_semaphore_wait_n(mx_os_semaphore *s, uint32_t count)
{
	do {
		sem_wait(s);
	} while (--count > 0);
}

static void mx_os_semaphore_signal_n(mx_os_semaphore *s, uint32_t count)
{
	do {
		sem_post(s);
	} while (--count > 0);
}

#endif

#endif
