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
	
	(void)argc;
	(void) argv;
	int input = ' ';
	
	while(input != 'q' && input != 'Q'){
		
		printf("\nWelcome to the cache simulator. Please select from the options below:\n\
				\n[Q]uit - Exits program.\n");
		input = getchar();
		
		
	
	}
	
	return 0;
}

//void 
