#include "mem-func.h"

/* 
 * Blok formaat:
 * 
 * unsigned short lengte_blok
 * unsigned short index_vorige_blok 	
 * 	(wijst naar lengte_blok long van vorige blok; 0 = eerste blok)
 * unsigned short index_volgend_blok 
 * 	(wijst naar lengte_blok long van vorige blok; 0 = laatste blok)
 * unsigned short vrij			
 * 	(1 = vrij, 0 = toegewezen)
 *
 * long[lengte_blok] data
 *
 */

void set_length(long index, long length){
	mem[index] &= 0x0000FFFFFFFFFFFF;
	mem[index] |= ((length & 0xFFFF) << 48);
}

void set_prev(long index, long prev){
	mem[index] &= 0xFFFF0000FFFFFFFF;
	mem[index] |= ((prev & 0xFFFF) << 32);
}

void set_next(long index, long next){
	mem[index] &= 0xFFFFFFFF0000FFFF;
	mem[index] |= ((next & 0xFFFF) << 16);
}

void set_free(long index, long free){
	mem[index] &= 0xFFFFFFFFFFFF0000;
	mem[index] |= free & 0xFFFF;
}

long get_length(long index){
	return (mem[index] >> 48) & 0xFFFF;
}

long get_prev(long index){
	return (mem[index] >> 32) & 0xFFFF;
}

long get_next(long index){
	return (mem[index] >> 16) & 0xFFFF;
}

long get_free(long index){
	return mem[index] & 0xFFFF;
}
