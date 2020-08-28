#ifndef MX_PLATFORM_EMSCRIPTEN_INCLUDED_H
#define MX_PLATFORM_EMSCRIPTEN_INCLUDED_H

#define MX_PLATFORM_EMSCRIPTEN

#include <stdint.h>
#include <emscripten/threading.h>

// -- Language

#define mx_forceinline inline __attribute__((always_inline))
#define mx_noinline __attribute__((noinline))
#define mx_unreachable() __builtin_trap()
#define mx_assert(cond) do { if (!(cond)) __builtin_trap(); } while (0)

// -- mxa_*

// -- mxa_*32

#define mxa_load32(src) emscripten_atomic_load_u32(src)
#define mxa_or32(dst, val) emscripten_atomic_or_u32((dst), (val))
#define mxa_and32(dst, val) emscripten_atomic_and_u32((dst), (val))
#define mxa_inc32(dst) emscripten_atomic_add_u32((dst), 1)
#define mxa_dec32(dst) emscripten_atomic_sub_u32((dst), 1)
#define mxa_add32(dst, val) emscripten_atomic_add_u32((dst), (val))
#define mxa_sub32(dst, val) emscripten_atomic_sub_u32((dst), (val))
#define mxa_cas32(dst, cmp, val) (emscripten_atomic_cas_u32((dst), (cmp), (val)) == (cmp))
#define mxa_exchange32(dst, val) emscripten_atomic_exchange_u32((dst), (val))

#define mxa_load32_acq(src) mxa_load32(src)
#define mxa_or32_acq(dst, val) mxa_or32(dst, val)
#define mxa_and32_acq(dst, val) mxa_and32(dst, val)
#define mxa_inc32_acq(dst) mxa_inc32(dst)
#define mxa_dec32_acq(dst) mxa_dec32(dst)
#define mxa_add32_acq(dst, val) mxa_add32(dst, val)
#define mxa_sub32_acq(dst, val) mxa_sub32(dst, val)
#define mxa_cas32_acq(dst, cmp, val) mxa_cas32(dst, cmp, val)
#define mxa_exchange32_acq(dst, val) mxa_exchange32(dst, val)

#define mxa_load32_rel(src) mxa_load32(src)
#define mxa_or32_rel(dst, val) mxa_or32(dst, val)
#define mxa_and32_rel(dst, val) mxa_and32(dst, val)
#define mxa_inc32_rel(dst) mxa_inc32(dst)
#define mxa_dec32_rel(dst) mxa_dec32(dst)
#define mxa_add32_rel(dst, val) mxa_add32(dst, val)
#define mxa_sub32_rel(dst, val) mxa_sub32(dst, val)
#define mxa_cas32_rel(dst, cmp, val) mxa_cas32(dst, cmp, val)
#define mxa_exchange32_rel(dst, val) mxa_exchange32(dst, val)

#define mxa_load32_nf(src) mxa_load32(src)
#define mxa_or32_nf(dst, val) mxa_or32(dst, val)
#define mxa_and32_nf(dst, val) mxa_and32(dst, val)
#define mxa_inc32_nf(dst) mxa_inc32(dst)
#define mxa_dec32_nf(dst) mxa_dec32(dst)
#define mxa_add32_nf(dst, val) mxa_add32(dst, val)
#define mxa_sub32_nf(dst, val) mxa_sub32(dst, val)
#define mxa_cas32_nf(dst, cmp, val) mxa_cas32(dst, cmp, val)
#define mxa_exchange32_nf(dst, val) mxa_exchange32(dst, val)

// -- mxa_*_ptr

#define mxa_load_ptr(src) (void*)mxa_load32(src)
#define mxa_cas_ptr(dst, cmp, val) mxa_cas32(dst, (uint32_t)(cmp), (uint32_t)(val))
#define mxa_exchange_ptr(dst, val) mxa_exchange32((uint32_t*)(dst), (uint32_t)val)

#define mxa_load_ptr_acq(src) mxa_load_ptr(src)
#define mxa_cas_ptr_acq(dst, cmp, val) mxa_cas_ptr(dst, cmp, val)
#define mxa_exchange_ptr_acq(dst, val) mxa_exchange_ptr(dst, val)

#define mxa_load_ptr_rel(src) mxa_load_ptr(src)
#define mxa_cas_ptr_rel(dst, cmp, val) mxa_cas_ptr(dst, cmp, val)
#define mxa_exchange_ptr_rel(dst, val) mxa_exchange_ptr(dst, val)

#define mxa_load_ptr_nf(src) mxa_load_ptr(src)
#define mxa_cas_ptr_nf(dst, cmp, val) mxa_cas_ptr(dst, cmp, val)
#define mxa_exchange_ptr_nf(dst, val) mxa_exchange_ptr(dst, val)

// -- Misc intrinsics

#define mx_ctz32(mask) (uint32_t)__builtin_ctz((unsigned int)(mask))

// -- Threads

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

typedef uint32_t mx_os_semaphore;

static void mx_os_semaphore_init(mx_os_semaphore *s)
{
	*s = 0;
}

static void mx_os_semaphore_free(mx_os_semaphore *s)
{
}

static void mx_os_semaphore_wait(mx_os_semaphore *s)
{
	for (;;) {
		uint32_t v = *s;
		if (v > 0 && mxa_cas32(s, v, v - 1)) {
			break;
		} else {
			emscripten_futex_wait(s, 0, 1000.0);
		}
	}
}

static void mx_os_semaphore_signal(mx_os_semaphore *s)
{
	uint32_t v;
	do {
		v = *s;
	} while (!mxa_cas32(s, v, v + 1));
	emscripten_futex_wake(s, 1);
}

static void mx_os_semaphore_wait_n(mx_os_semaphore *s, uint32_t count)
{
	uint32_t left = count;
	for (;;) {
		uint32_t v = *s;
		uint32_t sub = left < v ? left : v;
		if (v > 0 && mxa_cas32(s, v, v - sub)) {
			left -= sub;
			if (left == 0) break;
		} else {
			emscripten_futex_wait(s, 0, 1000.0);
		}
	}
}

static void mx_os_semaphore_signal_n(mx_os_semaphore *s, uint32_t count)
{
	uint32_t v;
	do {
		v = *s;
	} while (!mxa_cas32(s, v, v + count));
	emscripten_futex_wake(s, count);
}

#endif
