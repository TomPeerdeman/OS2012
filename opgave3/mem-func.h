#ifndef MEM_FUNC_H
#define MEM_FUNC_H

/* 
 * Blok formaat:
 * 
 * long lengte_blok
 * long index_vorige_blok 	
 * 	(wijst naar lengte_blok long van vorige blok; 0 = eerste blok)
 * long index_volgend_blok 
 * 	(wijst naar lengte_blok long van vorige blok; 0 = laatste blok)
 * long vrij			
 * 	(1 = vrij, 0 = toegewezen)
 * long[lengte_blok] data
 *
 */

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

void print_blocks();

 /* Lengte van een administratie blok. Aantal longs vorafgaande de eigenlijke
  * data. */
 #define ADMIN_SIZE 4

/* Pointer naar geheugen dat gebruikt wordt. */
long *mem;

#endif
