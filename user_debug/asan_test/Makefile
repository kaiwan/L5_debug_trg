# Makefile, Ch 5
#--------------------------------------------------------------
# This program is part of the source code released for the book
#  "Hands-on System Programming with Linux"
#  (c) Author: Kaiwan N Billimoria
#  Publisher:  Packt
#
# From: Ch 5 : Linux Memory Issues
#----------------------------------------------------------------------
#  ASSUMPTIONS ::
#   1. the convenience files ../common.h and ../common.c
#      are present
#   2. the clang/LLVM compiler is installed
#   3. the indent utility is installed
#
#   WARNING! Do NOT start a source filename with 'core' !
#       (will get Erased when 'make clean' is performed).
#----------------------------------------------------------------------
## Pl check and keep or remove <foo>_dbg_[asan|ub|msan] targets
## as desired. We have deliberately not kept the sanitizer targets; add
## them to the 'ALL := ' statement if you wish to build them by default.

ALL :=  membugs membugs_dbg membugs_dbg_asan membugs_dbg_msan membugs_dbg_ub

#CC=${CROSS_COMPILE}gcc
CC=${CROSS_COMPILE}clang

CFLAGS=-O2 -Wall -Wextra -UDEBUG
CFLAGS_DBG=-g -O0 -Wall -Wextra -DDEBUG
CFLAGS_DBG_ASAN=${CFLAGS_DBG} -fsanitize=address -fsanitize-address-use-after-scope
CFLAGS_DBG_MSAN=${CFLAGS_DBG} -fsanitize=memory
CFLAGS_DBG_UB=${CFLAGS_DBG} -fsanitize=undefined

LINKIN := -static-libasan
 # user will need to explicitly set libraries to link in as required;
 # f.e. -lrt -pthread
 # TODO : On Fedora with clang, the directive should be : -static-libsan
 # (clang doesn't require the lib to be statically linked?)

all: ${ALL}
CB_FILES := *.[ch]

membugs: membugs.c
	${CC} ${CFLAGS} membugs.c -o membugs
	strip --strip-all ./membugs
membugs_dbg: membugs.c
	${CC} ${CFLAGS_DBG} membugs.c -o membugs_dbg

 #--- Sanitizers for membugs :: (use clang): <foo>_dbg_[asan|ub|msan]
membugs_dbg_asan: membugs.c
	${CC} ${CFLAGS_DBG_ASAN} membugs.c -o membugs_dbg_asan #-static-libasan
membugs_dbg_msan: membugs.c
	clang ${CFLAGS_DBG_MSAN} membugs.c -o membugs_dbg_msan
membugs_dbg_ub: membugs.c
	${CC} ${CFLAGS_DBG_UB} membugs.c -o membugs_dbg_ub

# indent- "beautifies" C code into the "Linux kernel style".
# (cb = C Beautifier :) )
# Note! original source file(s) is overwritten, so we back it up.
cb: ${CB_FILES}
	mkdir bkp 2> /dev/null; cp -f ${CB_FILES} bkp/
	indent -linux ${CB_FILES}
clean:
	rm -vf ${ALL} core* vgcore* *.o *~
