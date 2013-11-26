CC = gcc
CFLAGS = -c -g -Wall -Wextra
LFLAGS = -g -Wall -Wextra


.PHONY: all clean

all: memsim

memsim: memsim.o
	$(CC) $(LFLAGS) $^ -o $@ -lm

memsim.o: memsim.c
	$(CC) $(CFLAGS) $<
	
clean:
	rm -f memsim
	rm -f *.o
	rm -f *~
