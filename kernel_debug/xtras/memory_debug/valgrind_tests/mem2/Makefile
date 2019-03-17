# Makefile
# For demo program mem2.c -- memory bugs tester
# # (we intend to run valgrind on it).

all: mem2 3mem
mem2: mem2.c
	gcc -g -O0 mem2.c -o mem2 -Wall
3mem: 3mem.c
	gcc -g -O0 3mem.c -o 3mem -Wall

clean:
	rm -f mem2 3mem

