# Makefile
# For demo program allmemtests.c -- memory bugs tester
# # (we intend to run valgrind on it).
# allmemtests.c (c) Cal Erickson.

ALL: allmemtests leak_test
all: ${ALL}

allmemtests: allmemtests.c
	gcc -g -ggdb -O0 allmemtests.c -o allmemtests -Wall
leak_test: leak_test.c
	gcc -g -ggdb -O0 leak_test.c -o leak_test -Wall

val1:
	valgrind --leak-check=full ./leak_test 50 3
val2:
	valgrind --leak-check=full ./allmemtests 0

clean:
	rm -f ${ALL}

