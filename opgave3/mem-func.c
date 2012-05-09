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
		printf("Block at idx %ld; length: %ld; free: %ld; prev: %ld; next: %ld\n", idx, get_length(idx), get_free(idx), get_prev(idx), get_next(idx));
		idx = get_next(idx);
	}
}

void new_block(long index, long length, long prev, long next){
	printf("New block at %ld; length: %ld; prev: %ld; next: %ld\n", index, length, prev, next);
	set_length(index, length);
	set_prev(index, prev);
	set_next(index, next);
	set_free(index, 1);
}

int split_block(long index, long length){
	long blockleng = get_length(index);
	long newidx = index + length + ADMIN_SIZE;
	long newleng = blockleng - length - ADMIN_SIZE;
	
	printf("Split %ld at %ld\n", blockleng, length);
	if(blockleng < length + ADMIN_SIZE + 1){
		return -1;
	}

	new_block(newidx, newleng, index, get_next(index));
	
	if(get_next(index) != 0){
		set_prev(get_next(index), newidx);
	}
	set_next(index, newidx);
	set_length(index, length);
	set_free(index, 0);
	mem[1] += ADMIN_SIZE;
	
	return index + ADMIN_SIZE;
}