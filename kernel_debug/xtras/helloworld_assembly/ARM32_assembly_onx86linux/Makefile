# Makefile
# ForARM-32 assembly
ALL := helloarm

# We assume you have the '${CROSS_COMPILE}' x86-to-ARM32 cross toolchain installed
CROSS_COMPILE=arm-none-linux-gnueabi-
AS=${CROSS_COMPILE}as
LD=${CROSS_COMPILE}ld
CFLAGS=-c -Wall

all: ${ALL}

helloarm: hello.o
	${LD} -o helloarm hello_ARM32.o
hello.o : hello_ARM32.s
	${AS} -o hello_ARM32.o hello_ARM32.s

clean:
	rm -fv ${ALL} *.o
