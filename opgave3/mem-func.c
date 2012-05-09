#include "mem-func.h"

#include <stdio.h>

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

void print_blocks(){
	long idx = 2;
	while(idx != 0){
		printf("Block at idx %ld; length: %ld; free: %ld; prev: %ld; next:"
			"%ld\n", idx, get_length(idx), get_free(idx),
			get_prev(idx), get_next(idx));
		
		idx = get_next(idx);
	}
}

/* Deze methode maakt een nieuw vrij blok aan met als eigenschappen de
 * parameters van de methode. */
void new_block(long index, long length, long prev, long next){
	printf("New block at %ld; length: %ld; prev: %ld; next: %ld\n", 
		index, length, prev, next);
	
	set_length(index, length);
	set_prev(index, prev);
	set_next(index, next);
	set_free(index, 1);
}

/* Deze methode wijst length woorden toe aan een proces. De overgebleven
 * woorden worden gezien als nieuw vrij blok. */
int split_block(long index, long length){
	long blockleng = get_length(index);
	long newidx = index + length + ADMIN_SIZE;
	long newleng = blockleng - length - ADMIN_SIZE;
	
	printf("Split %ld at %ld\n", blockleng, length);
	if(blockleng < length + ADMIN_SIZE + 1){
		/* Geen ruimte voor een nieuw blok van minimaal 1 woord. */
		return -1;
	}
	
	/* Maak het nieuwe blok. Plaats deze na 'length' woorden. */
	new_block(newidx, newleng, index, get_next(index));
	
	/* Als het huidige blok een volgende blok heeft moet de pointer van
	 * dat blok welke naar zijn vorige blok wijst naar het nieuwe blok
	 * gezet worden.*/
	if(get_next(index) != 0){
		set_prev(get_next(index), newidx);
	}
	/* Zet het volgende blok van het huidige blok naar het nieuwe blok. */
	set_next(index, newidx);
	
	/* Zet de length van het huidige blok en zet hem op toegewezen. */
	set_length(index, length);
	set_free(index, 0);
	
	/* Verhoog het aantal loze woorden. */
	mem[1] += ADMIN_SIZE;
	/* Verhoog het aantal toegewezen woorden. */
	mem[0] += length;
	
	/* De index waar begonnen mag worden met schrijven is het blok index
	 * plus de lengte van de administratie. */
	return index + ADMIN_SIZE;
}

/* Deze methode zet het blok op index op vrij, indien mogelijk fuseert het
 * met omringende vrije blokken. */
void free_block(long index){
	long prev = get_prev(index);
	long next = get_next(index);
	
	if(!get_free(index)){
		/* Zet het blok op vrij. */
		set_free(index, 1);
		mem[0] -= get_length(index);
	}
	
	/* Voeg vorige blok samen met het huidige als deze vrij is als een groot
	 * vrij blok. */
	if(prev != 0 && get_free(prev)){
		printf("Fuse %ld with %ld\n", index, prev);
		set_length(prev, get_length(prev) + get_length(index) + ADMIN_SIZE);
		set_next(prev, next);
		
		if(next != 0){
			set_prev(next, prev);
		}
		
		mem[1] -= ADMIN_SIZE;
	}
	
	/* Voeg volgende blok samen met het huidige als deze vrij is als een 
	 * groot vrij blok. */
	if(next != 0 && get_free(next)){
		free_block(next);
	}
}