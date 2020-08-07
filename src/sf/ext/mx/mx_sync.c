#include "mx_sync.h"
#include "mx_platform.h"

#ifndef MX_SYNC_DEFAULT_SPIN
#define MX_SYNC_DEFAULT_SPIN 500
#endif

#include <stdlib.h>
#include <string.h>

#ifndef mx_malloc
#define mx_malloc(size) malloc(size)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// -- Configuration

static uint32_t g_default_spin = MX_SYNC_DEFAULT_SPIN;

void mx_sync_set_default_spin(uint32_t spin)
{
	g_default_spin = spin;
}

// -- Semaphore pool
// Global semaphore pool that supports lock-free allocation and recycling
// of OS semaphores. Pooled semaphores are identified by non-zero `uint32_t`
// handles. Semaphores are kept in a linked list of chunks of 32 semaphores.

typedef struct sema_block sema_block;
struct sema_block {
	mx_os_semaphore semaphores[32]; // < OS Semaphore handles, initialized to zero
	uint32_t free_mask;             // < Bit N=1 means `semaphores[N]` is free for re-use
	uint32_t init_mask;             // < Bit N=1 means `semaphores[N]` is initialized
	sema_block *next;               // < Link to next block
	void *memory;                   // < Pointer to the unaligned memory to free
};

static sema_block g_sema_root;

// Allocate a semaphore from the pool
uint32_t mx_sema_pool_alloc()
{
	sema_block *block = &g_sema_root;
	uint32_t base = 1; // < Semaphore indices are 1-based
	for (;;) {
		uint32_t mask;

		// Try to re-use a freed semaphore
		while ((mask = mxa_load32_nf(&block->free_mask)) != 0) {
			uint32_t bit_ix = mx_ctz32(mask);
			uint32_t bit_mask = UINT32_C(1) << bit_ix;
			uint32_t clear_mask = ~bit_mask;
			if (mxa_and32_acq(&block->free_mask, clear_mask) & bit_mask) {
				// block->free_mask: ^^^ block->semaphores[bit_ix] ^^^
				return base + bit_ix;
			}
		}

		// Try to allocate a new semaphore from this block.
		// Note that the mask is negated as 0 means not initialized!
		while ((mask = ~ mxa_load32_nf(&block->init_mask)) != 0) {
			uint32_t bit_ix = mx_ctz32(mask);
			uint32_t bit_mask = UINT32_C(1) << bit_ix;
			if (~ mxa_or32_acq(&block->init_mask, bit_mask) & bit_mask) {
				// block->init_mask: ^^^ block->semaphores[bit_ix] ^^^
				mx_os_semaphore_init(&block->semaphores[bit_ix]);
				return base + bit_ix;
			}
		}

		// Advance to (or try to create) the next block
		sema_block *next = (sema_block*)mxa_load_ptr_acq(&block->next);
		// ^^^ block->next ^^^
		if (next == NULL) {
			// Allocate aligned to 128 bytes to not trash unrelated cache lines
			char *data = (char*)mx_malloc(sizeof(sema_block) + 128);
			size_t align = (size_t)(-(intptr_t)data & 127);
			next = (sema_block*)(data + align);
			memset(next, 0, sizeof(sema_block));
			next->memory = data;

			// vvv block->next vvv
			if (!mxa_cas_ptr_rel(&block->next, NULL, next)) {
				// Failed to link, free the allocation and load
				// the actual next pointer.
				free(data);
				next = (sema_block*)mxa_load_ptr_acq(&block->next);
				// ^^^ block->next ^^^
			}
		}
		block = next;
		base += 32;
	}
}

static mx_forceinline sema_block *resolve_sema(uint32_t *p_ix)
{
	sema_block *block = &g_sema_root;
	uint32_t ix = *p_ix - 1; // < Indices are 1-based
	while (ix >= 32) {
		// No fence is needed as having `ix` fences the block
		block = mxa_load_ptr_nf(&block->next);
		ix -= 32;
	}
	*p_ix = ix;
	return block;
}

void mx_sema_pool_free(uint32_t ix)
{
	sema_block *block = resolve_sema(&ix);
	// block->free_mask: vvv block->semaphores[ix] vvv
	mxa_or32_rel(&block->free_mask, UINT32_C(1) << ix);
}

void mx_sema_pool_wait(uint32_t ix)
{
	sema_block *block = resolve_sema(&ix);
	mx_os_semaphore_wait(&block->semaphores[ix]);
}

void mx_sema_pool_signal(uint32_t ix)
{
	sema_block *block = resolve_sema(&ix);
	mx_os_semaphore_signal(&block->semaphores[ix]);
}

void mx_sema_pool_wait_n(uint32_t ix, uint32_t num)
{
	sema_block *block = resolve_sema(&ix);
	mx_os_semaphore_wait_n(&block->semaphores[ix], num);
}

void mx_sema_pool_signal_n(uint32_t ix, uint32_t num)
{
	sema_block *block = resolve_sema(&ix);
	mx_os_semaphore_signal_n(&block->semaphores[ix], num);
}

void mx_sema_pool_clean()
{
	sema_block *block = &g_sema_root;
	do {
		uint32_t mask;

		// Iterate through free semaphores and try to "allocate" them
		while ((mask = mxa_load32(&block->free_mask)) != 0) {
			uint32_t bit_ix = mx_ctz32(mask);
			uint32_t bit_mask = UINT32_C(1) << bit_ix;
			uint32_t clear_mask = ~bit_mask;
			if (mxa_and32_acq(&block->free_mask, clear_mask) & bit_mask) {
				// block->free_mask: ^^^ block->semaphroes[bit_ix] ^^^
				mx_os_semaphore_free(&block->semaphores[bit_ix]);
				// block->init_mask: vvv block->semaphroes[bit_ix] vvv
				mxa_and32_rel(&block->init_mask, clear_mask);
			}
		}

		block = (sema_block*)mxa_load_ptr_acq(&block->next);
		// ^^^ block->next ^^^
	} while (block != NULL);
}

// -- Pooled semaphore

// Lock/allocate pooled semaphore and increase refcount by refs
// Returns index into `sema_pool`
static mx_forceinline uint32_t pooled_sema_lock(mx_pooled_sema *ps, uint32_t refs)
{
	uint32_t sema;
	for (;;) {
		uint32_t s = mxa_load32_nf(&ps->state);
		if (s < 0x10000) {
			// Empty: Allocate one and initialize refcuont to `refs`
			sema = mx_sema_pool_alloc();

			// ps->state: vvv sema vvv
			if (mxa_cas32_rel(&ps->state, s, sema << 16u | refs)) {
				// OK: Empty -> Sema (N refs)
				break;
			}

			// Failed to transition, free the allocated semaphore
			mx_sema_pool_free(sema);
		} else {
			// Sema: Bump refcount by `refs`
			sema = s >> 16u;
			if (mxa_cas32_acq(&ps->state, s, s + refs)) {
				// ps->state: ^^^ sema ^^^
				// OK: Sema -> Sema (+N refs)
				break;
			}
		}
	}
	return sema;
}

// Unlock/free pooled semaphore and decrease refcount by refs
static mx_forceinline void pooled_sema_unlock(mx_pooled_sema *ps, uint32_t sema, uint32_t refs)
{
	// Reduce refcount by `refs`
	uint32_t s = mxa_sub32_nf(&ps->state, refs) - refs;

	// Try to free the semaphore if refcount reaches to zero
	if ((s & 0xffff) == 0) {
		// The only other valid transition from Sema state with zero refcount
		// is increasing the refcount so if this fails the semaphore will be
		// eventually freed by an another thread.
		if (mxa_cas32_nf(&ps->state, s, 0)) {
			// `sema` has been fenced already  at `pooled_sema_lock()`
			mx_sema_pool_free(sema);
		}
	}
}

void mx_pooled_sema_wait(mx_pooled_sema *ps)
{
	uint32_t sema = pooled_sema_lock(ps, 1u);
	mx_sema_pool_wait(sema);
	pooled_sema_unlock(ps, sema, 2u);
}

void mx_pooled_sema_signal(mx_pooled_sema *ps)
{
	uint32_t sema = pooled_sema_lock(ps, 2u);
	mx_sema_pool_signal(sema);
	pooled_sema_unlock(ps, sema, 1u);
}

void mx_pooled_sema_wait_n(mx_pooled_sema *ps, uint32_t num)
{
	uint32_t sema = pooled_sema_lock(ps, 1u * num);
	mx_sema_pool_wait_n(sema, num);
	pooled_sema_unlock(ps, sema, 2u * num);
}

void mx_pooled_sema_signal_n(mx_pooled_sema *ps, uint32_t num)
{
	uint32_t sema = pooled_sema_lock(ps, 2u * num);
	mx_sema_pool_signal_n(sema, num);
	pooled_sema_unlock(ps, sema, 1u * num);
}

// -- mx_mutex

static mx_noinline void mutex_lock_slow_spin(mx_mutex *m, uint32_t spin)
{
	for (;;) {
		uint32_t state = mxa_load32_nf(&m->state);
		if (state == 0) {
			// Free: Attempt to lock it
			if (mxa_cas32_acq(&m->state, 0, 1)) {
				// m->state: ^^^ ^^^
				// OK: Free -> Locked
				return;
			}
		} else if (state >= 0x10000) {
			// Semaphore: Try to increment refcount and join waiting
			if (mxa_cas32_acq(&m->state, state, state + 1)) {
				// OK: Semaphore -> Semaphore (+1 ref)
				mx_sema_pool_wait(state >> 16u);
				return;
			}
		} else if (state == 1) {
			if (spin > 0) {
				// Locked: Spin for a while and try to acquire the mutex
				// TODO: Adjust spin count based on time?
				do {
					// Stop spinning if the state is anything but Locked
					if (mxa_load32_nf(&m->state) != 1) break;
					mx_yield();
				} while (--spin > 0);
			} else {
				// Locked: Try to allocate and wait on a semaphore
				uint32_t sema = mx_sema_pool_alloc();
				// m->state: vvv sema vvv
				if (mxa_cas32_rel(&m->state, state, 1 | (sema << 16u))) {
					// OK: Locked -> Semaphore (1 ref)
					mx_sema_pool_wait(sema);
					return;
				} else {
					// Failed to transition, free the allocated semaphore
					mx_sema_pool_free(sema);
				}
			}
		} else {
			// Invalid state
			mx_unreachable();
		}
	}
}

static mx_noinline void mutex_lock_slow(mx_mutex *m)
{
	mx_mutex_lock_spin(m, g_default_spin);
}

static mx_noinline void mutex_unlock_slow(mx_mutex *m)
{
	for (;;) {
		uint32_t state = mxa_load32_nf(&m->state);
		if (state == 1) {
			// Locked: Try to unlock
			// m->state: vvv vvv
			if (mxa_cas32_rel(&m->state, 1, 0)) {
				// OK: Locked -> Free
				return;
			}
		} else if (state >= 0x10000) {
			// Semaphore: Check if there are threads waiting
			uint32_t sema = state >> 16u, num = state & 0xffffu;
			if (num > 0) {
				// One or more threads waiting on the semaphore, try to signal it
				if (mxa_cas32_acq(&m->state, state, state - 1)) {
					// m->state: ^^^ sema ^^^
					// OK: Semaphore -> Semaphore (-1 ref)
					mx_sema_pool_signal(sema);
					return;
				}
			} else {
				// The mutex is free but it still has a semaphore, try to free it
				if (mxa_cas32_acq(&m->state, state, 0)) {
					// m->state: ^^^ sema ^^^
					// OK: Semaphore -> Free
					mx_sema_pool_free(sema);
					return;
				}
			}
		} else if (state == 0) {
			// Trying to unlock an unlocked mutex
			mx_unreachable();
		} else {
			// Invalid state
			mx_unreachable();
		}
	}
}

void mx_mutex_lock(mx_mutex *m)
{
	if (!mxa_cas32_acq(&m->state, 0, 1)) {
		mutex_lock_slow(m);
	}
	// m->state: ^^^ ^^^
}

void mx_mutex_lock_spin(mx_mutex *m, uint32_t spin)
{
	if (!mxa_cas32_acq(&m->state, 0, 1)) {
		mutex_lock_slow_spin(m, spin);
	}
}

int mx_mutex_try_lock(mx_mutex *m)
{
	return mxa_cas32_acq(&m->state, 0, 1);
	// m->state: ^^^ ^^^
}

void mx_mutex_unlock(mx_mutex *m)
{
	// m->state: vvv vvv
	if (!mxa_cas32_rel(&m->state, 1, 0)) {
		mutex_unlock_slow(m);
	}
}

int mx_mutex_is_locked(const mx_mutex *m)
{
	return mxa_load32_acq(&m->state) != 0 ? 1 : 0;
	// m->state: ^^^ ^^^
}

// -- Recursive mutex

uint32_t mx_recursive_mutex_lock(mx_recursive_mutex *m)
{
	uint32_t thread_id = mx_get_thread_id();
	if (mx_mutex_try_lock(&m->mutex)) {
		m->thread_id = thread_id;
		return 1;
	} else if (m->thread_id == thread_id) {
		return ++m->recursion_depth + 1;
	}
	mutex_lock_slow(&m->mutex);
	m->thread_id = thread_id;
	return 1;
}

uint32_t mx_recursive_mutex_try_lock(mx_recursive_mutex *m)
{
	uint32_t thread_id = mx_get_thread_id();
	if (mx_mutex_try_lock(&m->mutex)) {
		m->thread_id = thread_id;
		return 1;
	} else if (m->thread_id == thread_id) {
		return ++m->recursion_depth + 1;
	} else {
		return 0;
	}
}

uint32_t mx_recursive_mutex_lock_spin(mx_recursive_mutex *m, uint32_t spin)
{
	uint32_t thread_id = mx_get_thread_id();
	if (mx_mutex_try_lock(&m->mutex)) {
		m->thread_id = thread_id;
		return 1;
	} else if (m->thread_id == thread_id) {
		return ++m->recursion_depth + 1;
	}
	mutex_lock_slow_spin(&m->mutex, spin);
	m->thread_id = thread_id;
	return 1;
}

uint32_t mx_recursive_mutex_unlock(mx_recursive_mutex *m)
{
	mx_assert(m->thread_id == mx_get_thread_id());
	if (m->recursion_depth > 0) {
		return m->recursion_depth--;
	} else {
		m->thread_id = 0;
		mx_mutex_unlock(&m->mutex);
		return 0;
	}
}

uint32_t mx_recursive_mutex_get_depth(const mx_recursive_mutex *m)
{
	if (mxa_load32_acq(&m->mutex.state) == 0) return 0;
	// m->state: ^^^ ^^^
	if (m->thread_id == mx_get_thread_id()) {
		return m->recursion_depth + 1;
	} else {
		return 0;
	}
}

// -- Semaphore

static mx_forceinline int semaphore_spin(mx_semaphore *s, uint32_t num, uint32_t spin)
{
	while (spin-- > 0) {
		int32_t count = (int32_t)mxa_load32_nf((uint32_t*)&s->count);
		if (count >= (int32_t)num) {
			if (mxa_cas32_acq((uint32_t*)&s->count, (uint32_t)count, (uint32_t)count - num)) {
				// s->count: ^^^ ^^^
				return 1;
			}
		}
		mx_yield();
	}
	return 0;
}

void mx_semaphore_wait(mx_semaphore *s)
{
	if (semaphore_spin(s, 1, g_default_spin)) return;
	int32_t count = (int32_t)mxa_dec32_acq((uint32_t*)&s->count) - 1;
	// s->count: ^^^ ^^^
	if (count < 0) {
		mx_pooled_sema_wait(&s->sema);
	}
}

void mx_semaphore_wait_spin(mx_semaphore *s, uint32_t spin)
{
	if (semaphore_spin(s, 1, spin)) return;
	int32_t count = (int32_t)mxa_dec32_acq((uint32_t*)&s->count) - 1;
	// s->count: ^^^ ^^^
	if (count < 0) {
		mx_pooled_sema_wait(&s->sema);
	}
}

int mx_semaphore_try_wait(mx_semaphore *s)
{
	int32_t count = (int32_t)mxa_load32_nf((uint32_t*)&s->count);
	if (count <= 0) return 0;
	return mxa_cas32_acq((uint32_t*)&s->count, (uint32_t)count, (uint32_t)(count - 1));
	// s->count: ^^^ ^^^
}

void mx_semaphore_signal(mx_semaphore *s)
{
	// s->count: vvv vvv
	int32_t count = (int32_t)mxa_inc32_rel((uint32_t*)&s->count);
	if (count < 0) {
		mx_pooled_sema_signal(&s->sema);
	}
}

void mx_semaphore_wait_n(mx_semaphore *s, uint32_t num)
{
	if (semaphore_spin(s, num, g_default_spin)) return;
	int32_t count = (int32_t)mxa_sub32_acq((uint32_t*)&s->count, num) - (int32_t)num;
	// s->count: ^^^ ^^^
	if (count < 0) {
		int32_t num_wait = -count <= (int32_t)num ? -count : (int32_t)num;
		mx_pooled_sema_wait_n(&s->sema, (uint32_t)num_wait);
	}
}

void mx_semaphore_wait_n_spin(mx_semaphore *s, uint32_t num, uint32_t spin)
{
	if (semaphore_spin(s, num, spin)) return;
	int32_t count = (int32_t)mxa_sub32_acq((uint32_t*)&s->count, num) - (int32_t)num;
	// s->count: ^^^ ^^^
	if (count < 0) {
		int32_t num_wait = -count <= (int32_t)num ? -count : (int32_t)num;
		mx_pooled_sema_wait_n(&s->sema, (uint32_t)num_wait);
	}
}

int mx_semaphore_try_wait_n(mx_semaphore *s, uint32_t num)
{
	int32_t count = (int32_t)mxa_load32_nf((uint32_t*)&s->count);
	if (count < (int32_t)num) return 0;
	return mxa_cas32_acq((uint32_t*)&s->count, (uint32_t)count, (uint32_t)count - num);
	// s->count: ^^^ ^^^
}

void mx_semaphore_signal_n(mx_semaphore *s, uint32_t num)
{
	// s->count: vvv vvv
	int32_t count = (int32_t)mxa_add32_rel((uint32_t*)&s->count, num);
	if (count < 0) {
		int32_t num_release = -count <= (int32_t)num ? -count : (int32_t)num;
		mx_pooled_sema_signal_n(&s->sema, (uint32_t)num_release);
	}
}

int32_t mx_semaphore_get_count(const mx_semaphore *s)
{
	return (int32_t)mxa_load32_acq((uint32_t*)&s->count);
	// s->count: ^^^ ^^^
}

// -- Read/write mutex

#define MAX_READERS (int32_t)(1 << 30)

void mx_rw_mutex_lock_read(mx_rw_mutex *m)
{
	int32_t count = (int32_t)mxa_inc32_acq(&m->num_readers) + 1;
	// m->num_readers: ^^^ ^^^
	if (count < 0) {
		mx_semaphore_wait(&m->reader_sem);
	}
}

int mx_rw_mutex_try_lock_read(mx_rw_mutex *m)
{
	int32_t count = (int32_t)mxa_load32_nf(&m->num_readers);
	if (count < 0) return 0;
	return mxa_cas32_acq(&m->num_readers, (uint32_t)count, (uint32_t)count + 1);
	// m->num_readers: ^^^ ^^^
}

void mx_rw_mutex_unlock_read(mx_rw_mutex *m)
{
	int32_t count = (int32_t)mxa_dec32_nf(&m->num_readers) - 1;
	if (count < 0 && mxa_dec32_nf(&m->num_pending_readers) - 1 == 0) {
		mx_semaphore_signal(&m->writer_sem);
	}
}

void mx_rw_mutex_lock_write(mx_rw_mutex *m)
{
	mx_mutex_lock(&m->writer_mutex);
	int32_t count = (int32_t)mxa_sub32_nf(&m->num_readers, MAX_READERS);
	if (count > 0) {
		uint32_t num_wait = mxa_add32_nf(&m->num_pending_readers, (uint32_t)count) + (uint32_t)count;
		if (num_wait > 0) {
			mx_semaphore_wait(&m->writer_sem);
		}
	}
}

int mx_rw_mutex_try_lock_write(mx_rw_mutex *m)
{
	if (mxa_load32_nf(&m->num_readers) != 0) return 0;
	if (!mx_mutex_try_lock(&m->writer_mutex)) return 0;
	if (!mxa_cas32_nf(&m->num_readers, 0, (uint32_t)-MAX_READERS)) {
		mx_mutex_unlock(&m->writer_mutex);
		return 0;
	}
	return 1;
}

void mx_rw_mutex_unlock_write(mx_rw_mutex *m)
{
	// m->num_readers: vvv vvv
	int32_t count = (int32_t)mxa_add32_rel((uint32_t*)&m->num_readers, MAX_READERS) + MAX_READERS;
	if (count > 0) {
		mx_semaphore_signal_n(&m->reader_sem, count);
	}
	mx_mutex_unlock(&m->writer_mutex);
}

#ifdef __cplusplus
}
#endif
