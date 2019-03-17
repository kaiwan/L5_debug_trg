# Makefile
ALL := asan_try_dbg asan_try_rel

CC=gcc
CFLAGS_DBG=-fsanitize=address -D_REENTRANT -g -ggdb -gdwarf-4 -O0 -Wall
CFLAGS=-fsanitize=address -D_REENTRANT  -Wall

all: ${ALL}
CB_FILES := *.[ch]

asan_try_dbg: asan_try.c
	${CC} ${CFLAGS_DBG} asan_try.c -o asan_try_dbg #-static-libasan
asan_try_rel: asan_try.c
	${CC} ${CFLAGS} asan_try.c -o asan_try_rel -static-libasan

# cb - C Beautifier ! uses indent- "beautifies" C code into the "Linux kernel style"
# Note! original source file(s) is overwritten
cb: ${CB_FILES}
	mkdir bkp 2> /dev/null; cp -f ${CB_FILES} bkp/
	indent -linux ${CB_FILES}

clean:
	rm -vf ${ALL} *.o
