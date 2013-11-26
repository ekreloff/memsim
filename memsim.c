/*
 * Cache Memory Simulator
 * by Ethan Kreloff
 * 
 * Created November 25, 2013
 * 
 */

#include <stdio.h>

//Set to 1 to allow doing multiple simulations
#define INTERACTIVE 0

//cache constants in bytes
#define L1BLOCKSIZE 32
#define L2BLOCKSIZE 64
#define L2BUSWIDTH 16

//cache constants in cycles
#define L1HITTIME 1
#define L1MISSTIME 1

#define L2HITTIME 4
#define L2MISSTIME 6
#define L2TRANSFERTIME 6

#define MEMSENDADDR 10
#define MEMREADY 50
#define MEMCHUNKTIME 20

#define false 0
#define true 1
typedef int bool;


int main( int argc, const char* argv[] ){
	
	FILE *config_fp;
	int input = ' ';
	char op;
	unsigned long long int address;
	unsigned int bytesize;
	
	//Initialize parameters
	int l1_cache_size = 8192; 
	int l1_assoc = 1;
	int l2_cache_size = 65536;
	int l2_assoc = 1;
	int mem_chunk_size = 16;
	
	//Process config file to change defaults
	if((config_fp = fopen(argv[argc-1], "r")) != NULL){
	
	while(fscanf(config_fp, "%i %i %i %i %i", 
		  &l1_cache_size, &l1_assoc, &l2_cache_size, &l2_assoc, &mem_chunk_size) == 5){
		printf("\n%i %i %i %i %i\n", l1_cache_size, l1_assoc, l2_cache_size, l2_assoc, mem_chunk_size);
	  }
	}
	
	printf("\n%i %i %i %i %i\n", l1_cache_size, l1_assoc, l2_cache_size, l2_assoc, mem_chunk_size); 
	return 0;
	
	while (scanf("%c %Lx %d\n", &op, &address, &bytesize) == 3) {
		printf("\n%c %Lx %d", op, address, bytesize);
	}
	
	while(input != 'q' && input != 'Q' && INTERACTIVE){
		
		printf("\nWelcome to the cache simulator. Please select from the options below:\n\
				\n[Q]uit - Exits program.\n");
		input = getchar();
	
	}
	
	return 0;
}

//void simulate
