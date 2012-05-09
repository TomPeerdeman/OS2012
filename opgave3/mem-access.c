#include "mem-func.h"

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

void set_length(long index, long length){
	mem[index] = length;
}

void set_prev(long index, long prev){
	mem[index + 1] = prev;
}

void set_next(long index, long next){
	mem[index + 2] = next;
}

void set_free(long index, long free){
	mem[index + 3] = free;
}

long get_length(long index){
	return mem[index];
}

long get_prev(long index){
	return mem[index + 1];
}

long get_next(long index){
	return mem[index + 2];
}

long get_free(long index){
	return mem[index + 3];
}
