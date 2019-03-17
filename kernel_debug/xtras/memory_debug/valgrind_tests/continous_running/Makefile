# Makefile

CFLAGS = -Wall -Os
CFLAGS_DBG = -Wall -O0 -g -ggdb
OUT = cont_run_prcs_valgrindchk

all: ${OUT}

cont_run_prcs_valgrindchk: cont_run_prcs_valgrindchk.c
	gcc ${CFLAGS_DBG} cont_run_prcs_valgrindchk.c -o cont_run_prcs_valgrindchk

clean:
	rm ${OUT}

