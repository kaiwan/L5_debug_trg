# Makefile
# A 'better' Makefile template for Linux system programming
#
# Besides the 'usual' targets to build production and debug versions of the
# code and cleanup, we incorporate targets to do useful (and indeed required)
# stuff like:
#  - prod_2part: build a '2-part' production target 'prod_2part'; it's
#     -O2, no debug symbolic info, strip-debug;
#     Excellent for production as it gives ability to debug as and when required!
#  - indent: adhering to (Linux kernel) coding style guidelines (indent+checkpatch)
#  - sa: static analysis target (via flawfinder, cppcheck)
#  - dynamic analysis target: via valgrind
#  -         + code coverage via gcov
#  - a packaging (.tar.xz) target and
#  - a help target.
#
# You will require these utils installed:
#  indent, flawfinder, valgrind, kernel-headers package -or- simply the
#  checkpatch.pl script, gcov, tar; + libasan
#
# To get started, just type:
#  make help
#
# (c) 2020 Kaiwan N Billimoria, kaiwanTECH
# License: MIT

## Pl check and keep or remove <foo>_dbg_[asan|ub|msan] targets
## (where <foo> is the program name) as desired.
ALL :=  prod prod_2part

###
# Update as required
# Simply replace the variable ${FNAME_C} below (currently set to 'killer'),
# with your program name!
# Of course, if you have >1 C program to build, you must add it manually.
# Also, it's recommended to keep one Makefile per program in separate directories.
###
FNAME_C = stackmem_corrupt
ALL_NM :=  ${FNAME_C} ${FNAME_C}_dbg ${FNAME_C}_dbg_asan ${FNAME_C}_dbg_ub ${FNAME_C}_gcov

CC=${CROSS_COMPILE}gcc
CL=${CROSS_COMPILE}clang
STRIP=${CROSS_COMPILE}strip

PROD_OPTLEVEL=-O2
  # or -O3 or -Os
  # or -O3 or -Os
CSTD=-std=c18
POSIX_STD=201101 #199309L

CFLAGS=-Wall -pedantic -UDEBUG ${PROD_OPTLEVEL} -Werror=format-security ${CSTD} -D_POSIX_C_SOURCE=${POSIX_STD}
# Dynamic analysis includes the compiler itself!
# Especially the powerful Address Sanitizer (ASAN) toolset
CFLAGS_DBG=-g -ggdb -gdwarf-4 -O0 -Wall -pedantic -Wextra -DDEBUG -fno-omit-frame-pointer -Werror=format-security ${CSTD} -D_POSIX_C_SOURCE=${POSIX_STD}
CFLAGS_DBG_ASAN=${CFLAGS_DBG} -fsanitize=address
CFLAGS_DBG_UB=${CFLAGS_DBG} -fsanitize=undefined
#CFLAGS_DBG_MSAN=${CFLAGS_DBG} -fsanitize=memory

CFLAGS_GCOV=${CFLAGS_DBG} -fprofile-arcs -ftest-coverage

# Required vars
all: ${ALL}
SRC_FILES := *.[ch]
INDENT := indent
FLAWFINDER := flawfinder
CPPCHECK := cppcheck
VALGRIND := valgrind
# update as required
PKG_NAME := ${FNAME_C}
CHECKPATCH := /lib/modules/$(shell uname -r)/build/scripts/checkpatch.pl
 #../../checkpatch.pl

# Targets and their rules
# Three types:
# 1. 'regular' production target 'prod': -O2, no debug symbolic info, stripped
# 2. '2-part' production target 'prod_2part': -O2, no debug symbolic info, strip-debug;
#     excellent for production as it gives ability to debug as and when required!
#     (internally invokes the 'debug' target as it requires the debug binary as well
# 3. 'debug' target(s): -O0, debug symbolic info (-g -ggdb), not stripped
prod: ${FNAME_C}.c
	@echo
	@echo "--- building 'production'-ready target (-O2, no debug, stripped) ---"
	@echo
	${CC} ${CFLAGS} ${FNAME_C}.c -o ${FNAME_C}
	${STRIP} --strip-all ./${FNAME_C}

# The '2-part executable' solution : use strip and objcopy to generate a
# binary executable that has the ability to retrieve debug symbolic information
# from the 'debug' binary!
prod_2part: ${FNAME_C}.c
	@echo
	@echo "--- building 'production'-ready 2-part target (-O2, no debug, strip-debug) ---"
	@echo
# We require the 'debug' build for the 2part, so do that first
	make debug
	${CC} ${CFLAGS} ${FNAME_C}.c -o ${FNAME_C}
	${STRIP} --strip-debug ./${FNAME_C}
	objcopy --add-gnu-debuglink=./${FNAME_C}_dbg ./${FNAME_C}

debug: ${FNAME_C}.c
	@echo
	@echo "--- building 'debug'-ready targets (with debug symbolic info, not stripped) ---"
	@echo
	${CC} ${CFLAGS_DBG} ${FNAME_C}.c -o ${FNAME_C}_dbg
#-- Sanitizers (use clang or gcc)
	${CC} ${CFLAGS_DBG_ASAN} ${FNAME_C}.c -o ${FNAME_C}_dbg_asan
	${CC} ${CFLAGS_DBG_UB} ${FNAME_C}.c -o ${FNAME_C}_dbg_ub
#	${CC} ${CFLAGS_DBG_MSAN} ${FNAME_C}.c -o ${FNAME_C}_dbg_msan


#--------------- More (useful) targets! -------------------------------

# indent- "beautifies" C code - to conform to the the Linux kernel
# coding style guidelines.
# Note! original source file(s) is overwritten, so we back it up.
# code-style : "wrapper" target over the following kernel code style targets
code-style:
	make indent
	make checkpatch

indent: ${SRC_FILES}
	make clean
	@echo
	@echo "--- applying Linux kernel code-style indentation with indent ---"
	@echo
	mkdir bkp 2> /dev/null; cp -f ${SRC_FILES} bkp/
	${INDENT} -linux ${SRC_FILES}

checkpatch:
	make clean
	@echo
	@echo "--- applying Linux kernel code-style checking with checkpatch.pl ---"
	@echo
	${CHECKPATCH} -f --no-tree --max-line-length=95 ${SRC_FILES}

# sa : "wrapper" target over the following static analyzer targets
sa:   # static analysis
	make sa_flawfinder
	make sa_cppcheck

# static analysis with flawfinder
sa_flawfinder:
	make clean
	@echo
	@echo "--- static analysis with flawfinder ---"
	@echo
	${FLAWFINDER} --neverignore --context *.[ch]

# static analysis with cppcheck
sa_cppcheck:
	make clean
	@echo
	@echo "--- static analysis with cppcheck ---"
	@echo
	${CPPCHECK} -v --force --enable=all -i bkp/ --suppress=missingIncludeSystem .

# dynamic analysis with valgrind
valgrind:
	make debug
	@echo
	@echo "--- dynamic analysis with valgrind ---"
	@echo
	${VALGRIND} --tool=memcheck --trace-children=yes ./${FNAME_C}_dbg

# Testing: line coverage with gcov(1)
covg:
	@echo
	@echo "=== Code coverage with gcov ==="
	@echo
	${CC} ${CFLAGS_GCOV} ${FNAME_C}.c -o ${FNAME_C}_gcov
	./${FNAME_C}_gcov
	gcov ${SRC_FILES}
	@echo "Examine the *.c.gcov file(s)"

# packaging
package:
	@echo
	@echo "--- packaging ---"
	@echo
	rm -f ../${PKG_NAME}.tar.xz
	make clean
	tar caf ../${PKG_NAME}.tar.xz *
	ls -l ../${PKG_NAME}.tar.xz
	@echo "=== $(PKG_NAME).tar.xz package created ==="
	@echo 'Tip: when extracting, to extract into a dir of the same name as the tar file,'
	@echo ' do: tar -xvf ${PKG_NAME}.tar.xz --one-top-level'


clean:
	@echo
	@echo "--- cleaning ---"
	@echo
	rm -vf ${ALL_NM} core* vgcore* *.o *~ *.c.gcov *.gcda *.gcno

help:
	@echo '=== Makefile Help : additional targets available ==='
	@echo
	@echo 'TIP: type make <tab><tab> to show all valid targets'
	@echo

	@echo 'Regular targets ::'
	@echo ' 1. 'prod'  : regular production target: -O2, no debug symbolic info, stripped'
	@echo ' 2. 'debug' : -O0, debug symbolic info (-g -ggdb), not stripped'
	@echo ' 3. 'prod_2part': production target : -O2, no debug symbolic info, strip-debug; \
    Excellent for production as it gives ability to debug as and when required! \
    (shown as third option as it *requires* the 'debug' build'
	@echo
	@echo 'Doing a 'make' will build all three shown above.'

	@echo
	@echo '--- code style targets ---'
	@echo 'code-style : "wrapper" target over the following kernel code style targets'
	@echo ' indent     : run the $(INDENT) utility on source file(s) to indent them as per the kernel code style'
	@echo ' checkpatch : run the kernel code style checker tool on source file(s)'

	@echo
	@echo '--- static analyzer targets ---'
	@echo 'sa          : "wrapper" target over the following static analyzer targets'
	@echo ' sa_flawfinder : run the static analysis flawfinder tool on the source file(s)'
	@echo ' sa_cppcheck   : run the static analysis cppcheck tool on the source file(s)'

	@echo
	@echo '--- dynamic analysis targets ---'
	@echo ' valgrind   : run the dynamic analysis tool ($(VALGRIND)) on the binary executable'
	@echo ' covg       : run the gcov code coverage tool on the source'

	@echo
	@echo '--- misc targets ---'
	@echo ' clean      : cleanup - remove all the binaries, core files, etc'
	@echo ' package    : tar and compress the source files into the dir above'
	@echo '  Tip: when extracting, to extract into a dir of the same name as the tar file, do:'
	@echo '       tar -xvf ${PKG_NAME}.tar.xz --one-top-level'

	@echo ' help       : this 'help' target'
