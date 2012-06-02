#ifndef MEM_FUNC_H
#define MEM_FUNC_H

/* Lengte van een administratie blok. Aantal longs voorafgaande de eigenlijke
 * data. */

/* Amd 64 bit */
#ifdef __x86_64__
#define ADMIN_SIZE 1
#endif

#ifndef ADMIN_SIZE
#ifdef _M_X64
#define ADMIN_SIZE 1
#endif
#endif

/* Amd x86 bit */
#ifndef ADMIN_SIZE
#ifdef _M_X86
#define ADMIN_SIZE 2
#endif
#endif

/* Intel x86 */
#ifndef ADMIN_SIZE
#ifdef __i386__
#define ADMIN_SIZE 2
#endif
#endif

#ifndef ADMIN_SIZE
#ifdef _X86_
#define ADMIN_SIZE 2
#endif
#endif

/* Intel itanium 64 bit */
#ifndef ADMIN_SIZE
#ifdef __ia64__
#define ADMIN_SIZE 1
#endif
#endif

#ifndef ADMIN_SIZE
#define ADMIN_SIZE 2
#endif

void new_block(long index, long length, long prev, long next);

int split_block(long index, long leng);

void free_block(long index);

void set_length(long index, long length);

void set_prev(long index, long prev);

void set_next(long index, long next);

void set_free(long index, long free);

long get_length(long index);

long get_prev(long index);

long get_next(long index);

long get_free(long index);

/* Pointer naar geheugen dat gebruikt wordt. */
long *mem;

#endif
