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
#define INTERACTIVE 0

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
	unsigned int block_number;
	unsigned int LRU;
	bool dirty;
	bool valid;
	struct node *child;
} Node;


void printcache(Node cache);

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
		
	
	Node *current1, *setassocsearch1, *setassocsearch2, *fasearch1, *fasearch2 /*, *current2*/; 
	
	
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
	int search_number = 1;
	
	if(l1_assoc == 1){
		l1_index_bits = (log(L1BLOCKSIZE)/log(2)) + (log(l1_cache_size/L1BLOCKSIZE)/log(2));
	}else{if(l1_assoc == 0){
		l1_index_bits = (log(L1BLOCKSIZE)/log(2));
		}else{
			num_of_sets = (l1_cache_size/L1BLOCKSIZE)/l1_assoc;
			l1_index_bits = (log(L1BLOCKSIZE)/log(2)) + (log(num_of_sets)/log(2));
		}
	}
	
	printf("Number of sets: %i\n", num_of_sets);
	//this is for DM only, minor changes for other formats
	//int l2_index_bits = (log(L2BLOCKSIZE)/log(2)) + (log(l2_cache_size/L2BLOCKSIZE)/log(2));
	//unsigned long long int tagmask2 = (maskliteral << l2_index_bits) & maskliteral;
	/*unsigned long long int indexmask1 = ~tagmask1; 
	unsigned long long int indexmask2 = ~tagmask2;
	
	
	//printf("%i", sizeof(tagmask1));*/
	
	unsigned long long int maskliteral = 0xFFFFFFFFFFFF;
	unsigned long long int tagmask1 = (maskliteral << l1_index_bits) & maskliteral;
	
	unsigned int references;
	unsigned int counter = 0; 
	unsigned int addresscounter = 0;
	switch(l1_assoc){
		case 0:
		NEW_ADDRESS_FA:
	while (scanf("%c %Lx %d\n", &op, &address, &bytesize) == 3) {
		addresscounter++;
		printf("\nNEW ADRESS////////////////////////////////////%u\n", addresscounter);
		printf("index bits: %i tag mask 1: %#llX address: %#llX address after mask: %#llX\n",
			l1_index_bits, tagmask1, address, address & tagmask1);
		
		references = (int)(ceil((address%4 + bytesize)/4.0)); 
		printf("refs %u\n",references);
		address =  address - (address%4);
		counter = 0;
		
		NEW_WORD_FA:
		while(counter < references){ 
			if(op == 'I'){current1 = l1i_root;}else{current1 = l1d_root;}	
			while(current1 != NULL){
				//(current1->LRU)++;
					//printf("Checking if tags match\n");
					if(current1->tag == (address & tagmask1)){
						printf("current tag:%#llX address tag:%#llX\n", current1->tag, address & tagmask1);
						printf("Tags match, current address:%#llX\n", address);
						if(current1->valid){
							printf("Current is valid, setting current LRU to 0\n");
							current1->LRU = 0;
							printf("Cache Hit\n");
							if(op == 'W'){
								current1->dirty = true; printf("Write op: entry marked dirty\n");
								current1->address = address;
								current1->tag = address & tagmask1;
								current1->valid = true;
							}else{
								current1->valid = true;
							}
							if(!((counter + 1) < references)){
								printf("Last reference, going to new address\n");
								goto NEW_ADDRESS_FA;
							}else{
								printf("Next reference, going to new word\n");
								counter++;
								address += 4;
								goto NEW_WORD_FA;
							}
						}
					}
					//printf("checking if last search\n");
					if(search_number == (l1_cache_size/L1BLOCKSIZE)){
						printf("last search, highest LRU reset to zero\n");
					highest_LRU = 0;
					printf("Cache Miss\n");
					if(op == 'I'){fasearch1 = l1i_root;}else{fasearch1 = l1d_root;}
					while(fasearch1 != NULL){ 
						//printf("finding highest LRU, FASEARCHlru: %i, current highest: %i\n", fasearch1->LRU, highest_LRU);
						if(fasearch1->LRU > highest_LRU){
							printf("LRU higher and being replaced\n");
							highest_LRU = fasearch1->LRU;
						}else{fasearch1->LRU++;}
						fasearch1 = fasearch1->child;
					}
					
					if(op == 'I'){fasearch1 = l1i_root;}else{fasearch1 = l1d_root;}
					while(fasearch1 != NULL){ 
						//printf("deleting highest LRU, FASEARCHlru: %i, current highest: %i\n", fasearch1->LRU, highest_LRU);
						if(fasearch1->LRU >= highest_LRU){
							fasearch2 = fasearch1;
							printf("highest match LRU replaced: %i\n", fasearch2->LRU);
							break;
						}
						fasearch1 = fasearch1->child;
					}
					
					fasearch2->LRU = 0;
					
					
					if(fasearch2->dirty){
						printf("need to write to level 2\n");
					}
				
					printf("Need to return data from lower level memory\n");
					fasearch2->address = address;
					fasearch2->tag = address & tagmask1;
					fasearch2->valid = true;
					if(op == 'W'){
						fasearch2->dirty = true;
					}else{fasearch2->dirty = false;}
					if(!((counter + 1) < references)){
								goto NEW_ADDRESS_FA;
							}else{
								counter++;
								address += 4;
								goto NEW_WORD_FA;
							}
				}else{search_number++;}
				
					
				current1 = current1->child;
			}
			search_number = 1;
			counter++;
			address += 4;
		} 
	}
		break;
		case 1:
	NEW_ADDRESS:
	while (scanf("%c %Lx %d\n", &op, &address, &bytesize) == 3) {
		addresscounter++;		
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
				if(current1->block_number == address%(l1_cache_size/L1BLOCKSIZE)){
					printf("blocks match: %i %llu\n", current1->block_number, 
											  address%(l1_cache_size/L1BLOCKSIZE));
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
				if(current1->set_number == address%num_of_sets){
					printf("sets match: %i %llu\n", current1->set_number, 
											  address%num_of_sets);
					printf("current tag:%#llX address tag:%#llX\n", current1->tag, address & tagmask1);
					printf("current address:%#llX\n", address);
					if(current1->tag == (address & tagmask1)){
						if(current1->valid){
							current1->LRU = 0;
							printf("Cache Hit\n");
							if(op == 'W'){
								current1->dirty = true; printf("entry marked dirty!!!!!!!!!!!!!!!!!\n");
								current1->address = address;
								current1->tag = address & tagmask1;
								current1->valid = true;
							}else{
								current1->valid = true;
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
					highest_LRU = 0;
					printf("Cache Miss\n");
					printf("search Number: %i, %i\n", search_number, l1_assoc);
					if(search_number == l1_assoc){
					if(op == 'I'){setassocsearch1 = l1i_root;}else{setassocsearch1 = l1d_root;}
					while(setassocsearch1 != NULL){
						//printf("Set LRU: %i, highest: %i\n", setassocsearch1->LRU, highest_LRU); 
						if((setassocsearch1->LRU > highest_LRU) && ((address%num_of_sets) == setassocsearch1->set_number)){
							highest_LRU = setassocsearch1->LRU;
						}else{setassocsearch1->LRU++;}
						setassocsearch1 = setassocsearch1->child;
					}
					
					//printf("1: %i\n", highest_LRU);
					
					if(op == 'I'){setassocsearch1 = l1i_root;}else{setassocsearch1 = l1d_root;}
					while(setassocsearch1 != NULL){
						if((setassocsearch1->LRU >= highest_LRU) && ((address%num_of_sets) == setassocsearch1->set_number)){
							//printf("check sets in loop: %i, %i\n", (address%num_of_sets), setassocsearch1->set_number);
							setassocsearch2 = setassocsearch1;
							//printf("%i\n", setassocsearch2->set_number);
							//printf("2: %i, %i\n", highest_LRU, setassocsearch1->LRU);
							break;
						}
						setassocsearch1 = setassocsearch1->child;
					}
					
					if(op == 'I'){setassocsearch1 = l1i_root;}else{setassocsearch1 = l1d_root;}
					while(setassocsearch1 != NULL){
						if((address%num_of_sets) == setassocsearch1->set_number){
							//printf("3: %i, %i\n", highest_LRU, setassocsearch1->LRU);
						}
						setassocsearch1 = setassocsearch1->child;
					}
					setassocsearch2->LRU = 0;
					//printf("current tag:%#llX address tag:%#llX\n", setassocsearch2->tag, address & tagmask1);
						//printf("current address:%#llX\n", address);
					if(setassocsearch2->dirty){
						printf("need to write to level 2*************************\n");
						//printf("sets match: %i %llu\n", setassocsearch2->set_number, 
											  //address%num_of_sets);
					}
				
					printf("Need to return data from lower level memory\n");
					setassocsearch2->address = address;
					setassocsearch2->tag = address & tagmask1;
					setassocsearch2->valid = true;
					if(op == 'W'){
						setassocsearch2->dirty = true;
					}else{setassocsearch2->dirty = false;}
					printcache(*setassocsearch2);
					if(!((counter + 1) < references)){
								goto NEW_ADDRESS_SA;
							}else{
								counter++;
								address += 4;
								goto NEW_WORD_SA;
							}
				}else{search_number++;}
					
				}
				current1 = current1->child;
			}
			search_number = 1;
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

void printcache(Node cache){
	
	Node *temp = &cache;
		printf("Cache Address: %#llX Cache tag: %#llX Cache Set: %i\n\
		Cache Block: %i Cache LRU count: %i Dirty?: %i Valid?: %i \n\
		Next Cache Block: %i\n|\n|\n|\nV\n\n", temp->address, temp->tag, 
		temp->set_number, temp->block_number, temp->LRU, temp->dirty, 
		temp->valid, temp->child == NULL ? 0 : temp->child->block_number);
		
	
}


