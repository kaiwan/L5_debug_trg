# Makefile
# A 'better' Makefile template for Linux system programming.
#
# Meant for the case where a single C source file is built with various useful targets.
# If you require to compile/link multiple C source files into one executable, refer the
# <src>/makefile_templ/hdr_var/Makefile.
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
#  checkpatch.pl script, gcov, lcov, tar; + libasan
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
# We also provide the ../../mk script to trivially replace the value of FNAME_C.
# Of course, if you have >1 C program to build, you must add it manually.
# Also, it's recommended to keep one Makefile per program in separate directories.
###
FNAME_C := jg_quadeq_eg
#--- USER CHECK: manually add params as required
# Populate any required cmdline arguments to the process here:
CMDLINE_ARGS=


ALL_NM :=  ${FNAME_C} ${FNAME_C}_dbg ${FNAME_C}_dbg_asan ${FNAME_C}_dbg_ub ${FNAME_C}_gcov

CC=${CROSS_COMPILE}gcc
CL=${CROSS_COMPILE}clang
STRIP=${CROSS_COMPILE}strip
OBJCOPY=${CROSS_COMPILE}objcopy

PROD_OPTLEVEL=-O2
  # or -O3 or -Os
CSTD=-ansi -std=c99 -std=c11 -std=c18  # the last one wins; else if unsupported, earlier ones...
# For the meaning of the following feature test macros, see feature_test_macros(7)
POSIX_STD=201112L
STD_DEFS=-D_DEFAULT_SOURCE -D_GNU_SOURCE

# Add this option switch to CFLAGS / CFLAGS_DBG if you want ltrace to work on Ubuntu!
LTRACE_ENABLE=-z lazy

CFLAGS=-Wall -UDEBUG ${PROD_OPTLEVEL} -Werror=format-security ${CSTD} -D_POSIX_C_SOURCE=${POSIX_STD} ${STD_DEFS} -D_FORTIFY_SOURCE=2
# Dynamic analysis includes the compiler itself!
# Especially the powerful Address Sanitizer (ASAN) toolset
CFLAGS_DBG=-g -ggdb -gdwarf-4 -O0 -Wall -Wextra -DDEBUG -fno-omit-frame-pointer -Werror=format-security ${CSTD} -D_POSIX_C_SOURCE=${POSIX_STD} ${STD_DEFS} -D_FORTIFY_SOURCE=2
CFLAGS_DBG_ASAN=${CFLAGS_DBG} -fsanitize=address
CFLAGS_DBG_UB=${CFLAGS_DBG} -fsanitize=undefined
CFLAGS_DBG_MSAN=${CFLAGS_DBG} -fsanitize=memory

CFLAGS_GCOV=${CFLAGS_DBG} -fprofile-arcs -ftest-coverage
LINK=-lm  #-pthread

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
GCOV := gcov
LCOV := lcov
GENINFO := geninfo
GENHTML := genhtml

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
	@echo " glibc (and NPTL) version: $(shell getconf GNU_LIBPTHREAD_VERSION|cut -d' ' -f2)"
	@echo
	${CC} ${CFLAGS} ${FNAME_C}.c -o ${FNAME_C} ${LINK}
	${STRIP} --strip-all ./${FNAME_C}

# The '2-part executable' solution : use strip and objcopy to generate a
# binary executable that has the ability to retrieve debug symbolic information
# from the 'debug' binary!
prod_2part: ${FNAME_C}.c
	@echo
	@echo "--- building 'production'-ready 2-part target (-O2, no debug, strip-debug) ---"
	@echo " glibc (and NPTL) version: $(shell getconf GNU_LIBPTHREAD_VERSION|cut -d' ' -f2)"
	@echo
# We require the 'debug' build for the 2part, so do that first
	make --ignore-errors debug
	${CC} ${CFLAGS} ${FNAME_C}.c -o ${FNAME_C} ${LINK}
	${STRIP} --strip-debug ./${FNAME_C}
	${OBJCOPY} --add-gnu-debuglink=./${FNAME_C}_dbg ./${FNAME_C}

debug: ${FNAME_C}.c
	@echo
	@echo "--- building 'debug'-ready targets (with debug symbolic info, not stripped) ---"
	@echo " glibc (and NPTL) version: $(shell getconf GNU_LIBPTHREAD_VERSION|cut -d' ' -f2)"
	@echo
	${CC} ${CFLAGS_DBG} ${FNAME_C}.c -o ${FNAME_C}_dbg ${LINK}
#-- Sanitizers (use clang or GCC)
	${CC} ${CFLAGS_DBG_ASAN} ${FNAME_C}.c -o ${FNAME_C}_dbg_asan ${LINK} -static-libasan
	${CC} ${CFLAGS_DBG_UB} ${FNAME_C}.c -o ${FNAME_C}_dbg_ub ${LINK} -static-libasan
# GCC doesn't support MSAN, clang does
ifeq (, $(shell which clang))
	$(warning === WARNING! No clang (compiler) in PATH (reqd for MSAN); consider doing sudo apt install clang ===)
else
	${CL} ${CFLAGS_DBG_MSAN} ${FNAME_C}.c -o ${FNAME_C}_dbg_msan ${LINK}
endif


#--------------- More (useful) targets! -------------------------------

# indent- "beautifies" C code - to conform to the the Linux kernel
# coding style guidelines.
# Note! original source file(s) is overwritten, so we back it up.
# code-style : "wrapper" target over the following kernel code style targets
code-style:
	make --ignore-errors indent
	make --ignore-errors checkpatch

indent: ${SRC_FILES}
	make clean
	@echo
	@echo "--- applying Linux kernel code-style indentation with indent ---"
	@echo
	mkdir bkp 2>/dev/null; cp -f ${SRC_FILES} bkp/
	${INDENT} -linux ${SRC_FILES}
# RELOOK
# !WARNING!
# I came across this apparent bug in indent when using it on Ubuntu 20.04:
#  realloc(): invalid next size
#  Aborted (core dumped)
# Worse, it TRUNCATED the source file to 0 bytes !!! So backing them up - as we
# indeed do - is good.

checkpatch:
	make clean
	@echo
	@echo "--- applying Linux kernel code-style checking with checkpatch.pl ---"
	@echo
	${CHECKPATCH} -f --no-tree --max-line-length=95 ${SRC_FILES}

# sa : "wrapper" target over the following static analyzer targets
sa:   # static analysis
	make --ignore-errors sa_flawfinder
	make --ignore-errors sa_cppcheck

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

# Dynamic Analysis
# dynamic analysis with valgrind
valgrind:
	make --ignore-errors debug
	@echo
	@echo "--- dynamic analysis with Valgrind memcheck ---"
	@echo
#--- USER CHECK: have you populated CMDLINE_ARGS with the required cmdline
	${VALGRIND} --tool=memcheck --trace-children=yes ./${FNAME_C}_dbg ${CMDLINE_ARGS}

# dynamic analysis with the Sanitizer tooling
san:
	make --ignore-errors debug
	@echo
	@echo "--- dynamic analysis with the Address Sanitizer (ASAN) ---"
	@echo
#--- USER CHECK: have you populated CMDLINE_ARGS with the required cmdline
	./${FNAME_C}_dbg_asan ${CMDLINE_ARGS}

	@echo
	@echo "--- dynamic analysis with the Undefined Behavior Sanitizer (UBSAN) ---"
	@echo
#--- USER CHECK: have you populated CMDLINE_ARGS with the required cmdline
	./${FNAME_C}_dbg_ub ${CMDLINE_ARGS}

	@echo
	@echo "--- dynamic analysis with the Memory Sanitizer (MSAN) ---"
	@echo
#--- USER CHECK: have you populated CMDLINE_ARGS with the required cmdline
	./${FNAME_C}_dbg_msan ${CMDLINE_ARGS}

# Testing: line coverage with gcov(1), lcov(1)
# ref: https://backstreetcoder.com/code-coverage-using-gcov-lcov-in-linux/
covg:
	@echo
	@echo "=== Code coverage (funcs/lines/branches) with gcov+lcov ==="
	@echo

ifeq (,$(wildcard /etc/lcovrc))
	$(error ERROR: install lcov first)
endif
# Set up the ~/.lcovrc to include branch coverage
# ref: https://stackoverflow.com/questions/12360167/generating-branch-coverage-data-for-lcov
ifneq (,$(wildcard ~/.lcovrc))
	@echo "~/.lcovrc in place"
else
	cp /etc/lcovrc ~/.lcovrc
	sed -i 's/^#genhtml_branch_coverage = 1/genhtml_branch_coverage = 1/' ~/.lcovrc
	sed -i 's/^lcov_branch_coverage = 0/lcov_branch_coverage = 1/' ~/.lcovrc
endif

	${CC} ${CFLAGS_GCOV} ${FNAME_C}.c -o ${FNAME_C}_gcov ${LINK}
#--- USER CHECK: manually add params as required
# Update the test run below as required (more tests + params)
	@echo "= Running test case(s) ..."
	@echo "NOTE: in case the PUT (prg under test) runs continually, you'll have to terminate it"
	@echo " and then invoke the covg.sh script to complete the coverage analysis"
#--- USER CHECK: have you populated CMDLINE_ARGS with the required cmdline
	./${FNAME_C}_gcov ${CMDLINE_ARGS}
# ...As the PUT (prg under test) can run continually (without dying), it's possible
# that the instructions below will have to be run manually...
# Attempting to abort the PUT with ^C (or [p]kill) causes the entire chain - incl below-
# to abort. So, we need a separate script to run the stuff below; that's precisely
# the purpose behind the script <src>/covg.sh; run it if this code below doesn't run
# automatically.
	${GCOV} ${SRC_FILES}
# generate .info from the .gcno and .gcda file(s) in .
	${GENINFO} ./ -o ./${FNAME_C}.info
# generate HTML report in output directory lcov_html/
	${GENHTML} ./${FNAME_C}.info --output-directory lcov_html/
	@echo "Display lcov html report: google-chrome lcov_html/index.html"

# Testing all
# Limitation:
# When the PUT (Prg Under Test) runs in an infinite loop or forever (eg. servers/daemons),
# you may have to manually run a client process (or whatever) and exit the main process
# programatically; else, a signal like ^C does abort it BUT make doesn't continue (even
# when run with --ignore-errors).
test:
	@echo
	@echo "=== Test All ==="
	@echo "-------------------------------------------------------------------------------"
	make --ignore-errors code-style
	@echo "-------------------------------------------------------------------------------"
	make --ignore-errors sa
	@echo "-------------------------------------------------------------------------------"
	make --ignore-errors valgrind
	@echo "-------------------------------------------------------------------------------"
	make --ignore-errors san
	@echo "-------------------------------------------------------------------------------"
	make --ignore-errors covg

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
	rm -vf ${FNAME_C} ${FNAME_C}_dbg ${FNAME_C}_dbg_asan ${FNAME_C}_dbg_ub ${FNAME_C}_dbg_msan \
			${FNAME_C}_gcov core* vgcore* *.o *~
	rm -rfv *.[ch].gcov *.gcda *.gcno *.info lcov_html/

help:
	@echo '=== Makefile Help : additional targets available ==='
	@echo
	@echo 'This Makefile is appropriate for single-sourcefile program builds'
	@echo 'If your purpose is to build an app with multiple source files (across'
	@echo 'multiple dirs), then pl use the <src>/makefile_templ/hdr_var/Makefile template).'
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
	@echo ' san        : run dynamic analysis via ASAN, UBSAN and MSAN tooling on the binary executable'

	@echo
	@echo '--- code coverage ---'
	@echo ' covg       : run the gcov+lcov code coverage tooling on the source (generates html output!) NOTE- the covg.sh helper script is also available...'

	@echo
	@echo '--- TEST all ---'
	@echo ' test       : run all targets (it runs them in this order): code-style, sa, valgrind, san, covg'
	@echo '              Tip: run "make -i test > out 2>&1" to save all output to a file "out".'
	@echo '                             -i = --ignore-errors'

	@echo
	@echo '--- misc targets ---'
	@echo ' clean      : cleanup - remove all the binaries, core files, etc'
	@echo ' package    : tar and compress the source files into the dir above'
	@echo '  Tip: when extracting, to extract into a dir of the same name as the tar file, do:'
	@echo '       tar -xvf ${PKG_NAME}.tar.xz --one-top-level'

	@echo ' help       : this 'help' target'
