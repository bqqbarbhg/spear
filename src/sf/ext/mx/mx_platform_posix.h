#ifndef MX_PLATFORM_POSIX_INCLUDED_H
#define MX_PLATFORM_POSIX_INCLUDED_H

#include <stdint.h>
#include <string.h>
#include <semaphore.h>

// -- mx_os_semaphore

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
	sem_wait(s);
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
