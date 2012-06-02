/*
 * Bestand : first-fit.c
 *
 * Dit bestand bevat de implementatie voor de first-fit memory scheduler.
 *
 * Auteur: René Aparicio Saez
 * Student nr.: 10214054
 *
 * Auteur: Tom Peerdeman
 * Student nr.: 10266186
 *
 * Datum: 02/06/2012
 *
 */

#include "mem_alloc.h"
#include "mem-func.h"

void mem_init(long memory[MEM_SIZE]){
	mem = memory;
	
	/* Aantal toegewezen woorden. */
	mem[0] = 0;
	/* Aantal loze woorden. */
	mem[1] = 0;
	
	/* Eerste vrije blok. */
	new_block(2, (MEM_SIZE - ADMIN_SIZE - 2), 0, 0);
}

long  mem_get(long request){
	long l = request;
	long lidx = 0;
	long idx = 2;

	while(idx != 0){
		if(get_length(idx) >= l && get_free(idx)){
			/* Gat gevonden waar de aanvraag in past. */
			lidx = idx;
			l = get_length(idx);
			break;
		}
		
		idx = get_next(idx);
	}
	
	if(lidx == 0){
		/* Geen gat groot genoeg. */
		return -1;
	}else if(l == request){
		/* Gat precies groot genoeg. */
		mem[0] += request;
		mem[1] += ADMIN_SIZE;
		set_free(lidx, 0);
		return lidx + ADMIN_SIZE;
	}else{
		/* Alleen een gat > request. */
		return split_block(lidx, request);
	}
}

void mem_free(long index){
	free_block(index - ADMIN_SIZE);
}

double mem_internal(){
	/* NOTE: Wordt niet meer aangeroepen door de simulator. */
	return (double) mem[1] / (double) mem[0];
}

void mem_available(long *empty, long *large, long *n_holes){
	long idx = 2;
	long next;
	long prev;
	
	*empty = MEM_SIZE - mem[0] - mem[1];
	
	*large = 0;
	*n_holes = 0;
	while(idx != 0){
		next = get_next(idx);
		prev = get_prev(idx);
		/* Blok is een gat als het vrij is en de omringende blokken zijn
		 * niet vrij of bestaan niet (als dit blok het eerste of laatste
		 * blok is) */
		if(get_free(idx) && (
				(next && prev && !get_free(next) && !get_free(prev))
				|| (!prev && next && !get_free(next))
				|| (!next && prev && !get_free(prev))
				|| (!next && !prev)
			)
		){
			if(get_length(idx) > *large){
				/* Nieuw grootste gat gevonden. */
				*large = get_length(idx);
			}
			*n_holes += 1;
		}
		
		idx = get_next(idx);
	}
}

void mem_exit(){
}
