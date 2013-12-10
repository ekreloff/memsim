/*
 * Cache Memory Simulator
 * by Ethan Kreloff
 * 
 * Created November 25, 2013
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//Set to 1 to allow doing multiple simulations
//#define INTERACTIVE 0

//cache constants in bytes
#define L1BLOCKSIZE 32
#define L1BUSSIZE 4
#define L2BLOCKSIZE 64
#define L2BUSWIDTH 16

//In bits
#define ADDRSIZE 48

//cache constants in cycles
#define L1HITTIME 1
#define L1MISSTIME 1

#define L2HITTIME 4
#define L2MISSTIME 6
#define L2TRANSFERTIME 6

#define MEMSENDADDR 10
#define MEMREADY 50
#define MEMCHUNKTIME 20

#define true 1
#define false 0
typedef int bool;

typedef struct node{
	unsigned long long int address;
	unsigned long long int tag;
	unsigned int set_number;
	unsigned long long int block_number;
	unsigned int LRU;
	bool LRU_replacment;
	bool dirty;
	bool valid;
	struct node *child;
} Node;


int main( int argc, const char* argv[] ){
	
	FILE *config_fp;
	//int input = ' '; for interactive mode
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
			//printf("\n%i %i %i %i %i\n", l1_cache_size, l1_assoc, l2_cache_size, l2_assoc, mem_chunk_size);
	  }
	}
	
	int i; 
	unsigned int set_number = 0;
	Node *next, *l1d_root, *l1i_root, *l2_root;
	l1d_root = NULL;
	l1i_root = NULL;
	l2_root = NULL;
	
	for(i = 0; i < l1_cache_size/L1BLOCKSIZE; i++){
			next = (Node *)malloc(sizeof(Node));
			next->block_number = l1_cache_size/L1BLOCKSIZE - i - 1; 
			next->LRU = 0;
			next->LRU_replacment = false;
			next->dirty = false;
			next->valid = false;
			next->child = l1d_root;
			l1d_root = next;
			if(l1_assoc > 1){
				next->set_number = set_number;
				if((i%l1_assoc) == (l1_assoc - 1)){
					set_number++;
				}
			}
		}
	set_number = 0;
		
	for(i = 0; i < l1_cache_size/L1BLOCKSIZE; i++){
			next = (Node *)malloc(sizeof(Node));
			next->block_number = l1_cache_size/L1BLOCKSIZE - i - 1; 
			next->LRU = 0;
			next->LRU_replacment = false;
			next->dirty = false;
			next->valid = false;
			next->child = l1i_root;
			l1i_root = next;
			if(l1_assoc > 1){
				next->set_number = set_number;
				if((i%l1_assoc) == (l1_assoc - 1)){
					set_number++;
				}
			}
		}
	
	set_number = 0;
		
	for(i = 0; i < l2_cache_size/L2BLOCKSIZE; i++){
			next = (Node *)malloc(sizeof(Node));
			next->block_number = l2_cache_size/L2BLOCKSIZE - i - 1;
			next->LRU = 0;
			next->LRU_replacment = false;
			next->dirty = false;
			next->valid = false;
			next->child = l2_root;
			l2_root = next;
			if(l2_assoc > 1){
				next->set_number = set_number;
				if((i%l2_assoc) == (l2_assoc - 1)){
					set_number++;
				}
			}
		}
		

		/*Node *next1, *next2;
		next1 = l1_root;
		next2 = l2_root;
		i = 0;
		while(next1 != NULL){
			i++;
			//printf("\n%i %i", i, next->address);
			printf("\n%i %i", i, next1->block_number);
			next1 = next1->child; 
		
		}
		
		return 0;*/
	
	Node *current1, *setassocsearch1, *setassocsearch2 /*, *current2*/; 
	
	
	/*//Data being kept
	unsigned long long int execution_time = 0;
	unsigned long long int data_read_refs = 0;
	unsigned long long int data_write_refs = 0;
	unsigned long long int inst_refs = 0;
	unsigned long long int read_cycles = 0;
	unsigned long long int write_cycles = 0;
	unsigned long long int inst_cycles = 0;
	
	//Cache specific data
	unsigned long long int l1d_hit_count = 0;
	unsigned long long int l1d_miss_count = 0;
	unsigned long long int l1d_kickouts = 0;
	unsigned long long int l1d_dirty_kickouts = 0;
	unsigned long long int l1d_transfers = 0;
	
	unsigned long long int l1i_hit_count = 0;
	unsigned long long int l1i_miss_count = 0;
	unsigned long long int l1i_kickouts = 0;
	unsigned long long int l1i_dirty_kickouts = 0;
	unsigned long long int l1i_transfers = 0;
	
	unsigned long long int l2_hit_count = 01_index
	unsigned long long int l2_miss_count = 0;
	unsigned long long int l2_kickouts = 0;
	unsigned long long int l2_dirty_kickouts = 0;
	unsigned long long int l2_transfers = 0;
*/
	int l1_index_bits, num_of_sets;
	int highest_LRU = 0;
	
	if(l1_assoc == 1){
		l1_index_bits = (log(L1BLOCKSIZE)/log(2)) + (log(l1_cache_size/L1BLOCKSIZE)/log(2));
	}else{if(l1_assoc == 0){
		l1_index_bits = (log(L1BLOCKSIZE)/log(2));
		}else{
			num_of_sets = (l1_cache_size/L1BLOCKSIZE)/l1_assoc;
			l1_index_bits = (log(L1BLOCKSIZE)/log(2)) + (log(num_of_sets)/log(2));
		}
	}
	
	//this is for DM only, minor changes for other formats
	//int l2_index_bits = (log(L2BLOCKSIZE)/log(2)) + (log(l2_cache_size/L2BLOCKSIZE)/log(2));
	//unsigned long long int tagmask2 = (maskliteral << l2_index_bits) & maskliteral;
	/*unsigned long long int indexmask1 = ~tagmask1; 
	unsigned long long int indexmask2 = ~tagmask2;
	
	
	//printf("%i", sizeof(tagmask1));*/
	
	unsigned long long int maskliteral = 0xFFFFFFFFFFFF;
	unsigned long long int tagmask1 = (maskliteral << l1_index_bits) & maskliteral;
	unsigned long long int indexmask1 = (maskliteral >> (48 - l1_index_bits)) & 
										(maskliteral << (int)(log(L1BLOCKSIZE)/log(2)));
	
	unsigned int references;
	unsigned int counter = 0; 
	unsigned int addresscounter = 0;
	switch(l1_assoc){
		case 0:
		printf("FA l1 cache\n");
		break;
		case 1:
	NEW_ADDRESS:
	while (scanf("%c %Lx %d\n", &op, &address, &bytesize) == 3) {
		addresscounter++;
		//op == 'I' ? current1 = l1i_root : current1 = l1d_root; Oddly this doesnt
		// work and is equivalent to the code below.
		
		
		printf("\nNEW ADRESS////////////////////////////////////%u\n", addresscounter);
		printf("index bits: %i tag mask 1: %#llX address: %#llX address after mask: %#llX\n",
			l1_index_bits, tagmask1, address, address & tagmask1);
		
		references = (int)(ceil((address%4 + bytesize)/4.0)); 
		printf("refs %u\n",references);
		address =  address - (address%4);
		counter = 0;
		
		NEW_WORD:
		while(counter < references){ 
			if(op == 'I'){current1 = l1i_root;}else{current1 = l1d_root;}	
			while(current1 != NULL){
				//printf("current: %llu address block: %llu\n",current1->block_number , (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2)));
				if(current1->block_number == (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2))){
					printf("blocks match: %llu %llu\n", current1->block_number, 
											  (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2)));
					printf("current tag:%#llX address tag:%#llX\n", current1->tag, address & tagmask1);
					printf("current address:%#llX\n", address);
					if(current1->tag == (address & tagmask1)){
						if(current1->valid){
							printf("Cache Hit\n");
							if(op == 'W'){
								current1->dirty = true; printf("entry marked dirty\n");
								current1->address = address;
								current1->tag = address & tagmask1;
								current1->valid = true;
							}else{
								current1->valid = true;
							}
							if(!((counter + 1) < references)){
								goto NEW_ADDRESS;
							}else{
								counter++;
								address += 4;
								goto NEW_WORD;
							}
						}
					}
					printf("Cache Miss\n");
					if(current1->dirty){
						printf("need to write to level 2\n");
					}
				
					printf("Need to return data from lower level memory\n");
					current1->address = address;
					current1->tag = address & tagmask1;
					current1->valid = true;
					if(op == 'W'){
						current1->dirty = true;
					}else{current1->dirty = false;}
					
				}
				current1 = current1->child;
			}
			counter++;
			address += 4;
		} 
	}
	break;
	default:
	NEW_ADDRESS_SA:
	while (scanf("%c %Lx %d\n", &op, &address, &bytesize) == 3) {
		addresscounter++;
		
		printf("\nNEW ADRESS////////////////////////////////////%u\n", addresscounter);
		printf("index bits: %i tag mask 1: %#llX address: %#llX address after mask: %#llX\n",
			l1_index_bits, tagmask1, address, address & tagmask1);
		
		references = (int)(ceil((address%4 + bytesize)/4.0)); 
		printf("refs %u\n",references);
		address =  address - (address%4);
		counter = 0;
		
		NEW_WORD_SA:
		while(counter < references){ 
			if(op == 'I'){current1 = l1i_root;}else{current1 = l1d_root;}	
			while(current1 != NULL){
				if(current1->set_number == (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2))){
					printf("sets match: %i %llu\n", current1->set_number, 
											  (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2)));
					printf("current tag:%#llX address tag:%#llX\n", current1->tag, address & tagmask1);
					printf("current address:%#llX\n", address);
					for(i = 0; i < l1_assoc; i++){
					if(current1->tag == (address & tagmask1)){
						if(current1->valid){
							printf("Cache Hit\n");
							if(op == 'W'){
								current1->dirty = true; printf("entry marked dirty\n");
								current1->address = address;
								current1->tag = address & tagmask1;
								current1->valid = true;
							}else{
								current1->valid = true;
							}
							
							if(op == 'I'){setassocsearch1 = l1i_root;}else{setassocsearch1 = l1d_root;}
							while(setassocsearch1 != NULL){
								if(setassocsearch1->set_number == (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2))){
									for(i = 0; i < l1_assoc; i++){
										(setassocsearch1->LRU)++;
										setassocsearch1 = setassocsearch1->child;
									}
									break;
								}
								setassocsearch1 = setassocsearch1->child;
							}
							
							current1->LRU = 0;
							
							if(!((counter + 1) < references)){
								goto NEW_ADDRESS_SA;
							}else{
								counter++;
								address += 4;
								goto NEW_WORD_SA;
							}
						}
					}
					printf("Cache Miss\n");
					
					if(i == l1_assoc - 1){
					if(op == 'I'){setassocsearch1 = l1i_root;}else{setassocsearch1 = l1d_root;}
					while(setassocsearch1 != NULL){
						if(setassocsearch1->set_number == (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2))){
									for(i = 0; i < l1_assoc; i++){
										(setassocsearch1->LRU)++;
										setassocsearch1 = setassocsearch1->child;
									}
									break;
								}
						setassocsearch1 = setassocsearch1->child;
					}
					if(op == 'I'){setassocsearch1 = l1i_root;}else{setassocsearch1 = l1d_root;}
					while(setassocsearch1 != NULL){
						if(setassocsearch1->set_number == (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2))){
									//printf("replacing block: %i block LRU: %i highest lru: %i", setassocsearch1->block_number, setassocsearch1->LRU, highest_LRU);
									if(setassocsearch1->LRU > highest_LRU){
										highest_LRU = setassocsearch1->LRU;
									//printf("replacing block: %i block LRU: %i highest lru: %i\n", setassocsearch1->block_number, setassocsearch1->LRU, highest_LRU);
									}
								}
						setassocsearch1 = setassocsearch1->child;
					}
					if(op == 'I'){setassocsearch1 = l1i_root;}else{setassocsearch1 = l1d_root;}
					while(setassocsearch1 != NULL){
						if(setassocsearch1->set_number == (address & indexmask1) >> (int)(log(L1BLOCKSIZE)/log(2))){
									if(setassocsearch1->LRU == highest_LRU){
										setassocsearch2 = setassocsearch1;
									//	printf("replacing block: %i block LRU: %i highest lru: %i", setassocsearch2->block_number, setassocsearch2->LRU, highest_LRU);
										break;
									}
								}
						setassocsearch1 = setassocsearch1->child;
					}
					
					setassocsearch2->LRU = 0;
					printf("%i\n", setassocsearch2->block_number);
					
					if(setassocsearch2->dirty){
						printf("need to write to level 2\n");
					}
				
					printf("Need to return data from lower level memory\n");
					setassocsearch2->address = address;
					setassocsearch2->tag = address & tagmask1;
					setassocsearch2->valid = true;
					if(op == 'W'){
						setassocsearch2->dirty = true;
					}else{setassocsearch2->dirty = false;}
					
					if(i+1 < l1_assoc){
						current1 = current1->child;
					}
					
					if(!((counter + 1) < references)){
								goto NEW_ADDRESS_SA;
							}else{
								counter++;
								address += 4;
								goto NEW_WORD_SA;
							}
				}
				}
			    }
				current1 = current1->child;
			}
			counter++;
			address += 4;
		} 
	}
}
	return 0;
	
	/*while(input != 'q' && input != 'Q' && INTERACTIVE){
		
		printf("\nWelcome to the cache simulator. Please select from the options below:\n\
				\n[Q]uit - Exits program.\n");
		input = getchar();
	
	}
	
	return 0;*/
}


