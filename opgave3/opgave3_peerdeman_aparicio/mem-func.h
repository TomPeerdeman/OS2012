#ifndef MEM_FUNC_H
#define MEM_FUNC_H

/* Zet de gebruikte architectuur. (ARCH_0, ARCH_32, ARCH_64)*/
#define ARCH_32

/* Lengte van een administratie blok. Aantal longs voorafgaande de eigenlijke
 * data. */
#ifdef ARCH_0
#define ADMIN_SIZE 4
#endif

#ifdef ARCH_32
#define ADMIN_SIZE 2
#endif

#ifdef ARCH_64
#define ADMIN_SIZE 1
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
