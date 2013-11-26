/*
 * Cache Memory Simulator
 * by Ethan Kreloff
 * 
 * Created November 25, 2013
 * 
 */

#include <stdio.h>

#define false 0
#define true 1
typedef int bool;


int main( int argc, const char* argv[] ){
	
	FILE *config_fp;
	int input = ' ';
	char op;
	unsigned long long int address;
	unsigned int bytesize;
	
	//Setting default parameters
	int l1_block_size = 32;
	int l1_cache_size = 8192;
	int l1_assoc = 1;
	int l1_hit_time = 1;
	int l1_miss_time = 1;
	
	int l2_block_size = 64;
	int l2_cache_size = 65536;
	int l2_assoc = 1;
	int l2_hit_time = 4;
	int l2_miss_time = 6;
	int l2_transfer_time = 6;
	int l2_bus_width = 16;
	
	int mem_sendaddr = 10;
	int mem_ready = 50;
	int mem_chunktime = 20;
	int mem_chunksize = 16;
	
	//Process config file to change defaults
	config_fp = fopen(argv[argc-1], "r");
	
	while(fscanf(config_fp, "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i", 
		  &l1_block_size, &l1_cache_size, &l1_assoc, &l1_hit_time, &l1_miss_time,
		  &l2_block_size, &l2_cache_size, &l2_assoc, &l2_hit_time, &l2_miss_time, &l2_transfer_time, &l2_bus_width,
		  &mem_sendaddr, &mem_ready, &mem_chunktime, &mem_chunksize) == 16){
			
		printf("%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i", 
		l1_block_size, l1_cache_size, l1_assoc, l1_hit_time, l1_miss_time,
		l2_block_size, l2_cache_size, l2_assoc, l2_hit_time, l2_miss_time, l2_transfer_time, l2_bus_width,
		mem_sendaddr, mem_ready, mem_chunktime, mem_chunksize);
	  }
	  
	  return 0;
	
	while (scanf("%c %Lx %d\n", &op, &address, &bytesize) == 3) {
		printf("\n%c %Lx %d", op, address, bytesize);
	}
	
	while(input != 'q' && input != 'Q' && 0){
		
		printf("\nWelcome to the cache simulator. Please select from the options below:\n\
				\n[Q]uit - Exits program.\n");
		input = getchar();
	
	}
	
	return 0;
}

//void simulate
