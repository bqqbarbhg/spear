#ifndef MX_PLATFORM_SINGLETHREADED_INCLUDED_H
#define MX_PLATFORM_SINGLETHREADED_INCLUDED_H

#include <stdint.h>

// -- Language

#define mx_forceinline __attribute__((always_inline))
#define mx_noinline __attribute__((noinline))
#define mx_unreachable() __builtin_trap()
#define mx_assert(cond) do { if (!(cond)) __builtin_trap(); } while (0)

// -- mxa_*

// -- mxa_*32

#define mxa_load32(src) (*(const uint32_t*)(src))
static mx_forceinline uint32_t mxa_or32(uint32_t *dst, uint32_t val) {
	uint32_t prev = *dst; *dst |= val; return prev;
}
static mx_forceinline uint32_t mxa_and32(uint32_t *dst, uint32_t val) {
	uint32_t prev = *dst; *dst &= val; return prev;
}
static mx_forceinline uint32_t mxa_inc32(uint32_t *dst) {
	uint32_t prev = *dst; *dst += 1; return prev;
}
static mx_forceinline uint32_t mxa_dec32(uint32_t *dst) {
	uint32_t prev = *dst; *dst -= 1; return prev;
}
static mx_forceinline uint32_t mxa_add32(uint32_t *dst, uint32_t val) {
	uint32_t prev = *dst; *dst += val; return prev;
}
static mx_forceinline uint32_t mxa_sub32(uint32_t *dst, uint32_t val) {
	uint32_t prev = *dst; *dst -= val; return prev;
}
static mx_forceinline int mxa_cas32(uint32_t *dst, uint32_t cmp, uint32_t val) {
	if (*dst == cmp) {
		*dst = val;
		return 1;
	} else {
		return 0;
	}
}

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

#define mxa_load_ptr(src) (*(void *const *)(src))
static mx_forceinline int mxa_cas_ptr_imp(void **dst, void *cmp, void *val) {
	if (*dst == cmp) {
		*dst = val;
		return 1;
	} else {
		return 0;
	}
}
#define mxa_cas_ptr(ptr, cmp, val) mxa_cas_ptr_imp((void**)(ptr), (cmp), (val))
static mx_forceinline void *mxa_exchange_ptr_imp(void **dst, void *value) {
	void *tmp = *dst;
	*dst = value;
	return tmp;
}
#define mxa_exchange_ptr(ptr, val) mxa_exchange_ptr_imp((void**)(ptr), (val))

#define mxa_load_ptr_acq(src) mxa_load_ptr(src)
#define mxa_cas_ptr_acq(dst, cmp, val) mxa_cas_ptr(dst, cmp, val)
#define mxa_exchange_ptr_acq(ptr, val) mxa_exchange_ptr_imp((void**)(ptr), (val))

#define mxa_load_ptr_rel(src) mxa_load_ptr(src)
#define mxa_cas_ptr_rel(dst, cmp, val) mxa_cas_ptr(dst, cmp, val)
#define mxa_exchange_ptr_rel(ptr, val) mxa_exchange_ptr_imp((void**)(ptr), (val))

#define mxa_load_ptr_nf(src) mxa_load_ptr(src)
#define mxa_cas_ptr_nf(dst, cmp, val) mxa_cas_ptr(dst, cmp, val)
#define mxa_exchange_ptr_nf(ptr, val) mxa_exchange_ptr_imp((void**)(ptr), (val))

// -- Misc intrinsics

#define mx_ctz32(mask) (uint32_t)__builtin_ctz((unsigned int)mask)

// -- Threads

#define mx_get_thread_id() ((uint32_t)1)
#define mx_yield() (void)0

// -- mx_os_semaphore

typedef int mx_os_semaphore;

#define mx_os_semaphore_init(s)
#define mx_os_semaphore_free(s)

static mx_forceinline void mx_os_semaphore_wait(mx_os_semaphore *s)
{
	// Should never get here when singlethreaded
	mx_assert(0);
}

static mx_forceinline void mx_os_semaphore_signal(mx_os_semaphore *s)
{
	// Should never get here when singlethreaded
	mx_assert(0);
}

static mx_forceinline void mx_os_semaphore_wait_n(mx_os_semaphore *s, uint32_t count)
{
	// Should never get here when singlethreaded
	mx_assert(0);
}

static mx_forceinline void mx_os_semaphore_signal_n(mx_os_semaphore *s, uint32_t count)
{
	// Should never get here when singlethreaded
	mx_assert(0);
}

#endif
