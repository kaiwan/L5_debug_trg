/*
 * hello_ARM32.S
 * 'Hello, World' for ARM-32
 *
 * We can use QEMU user emulation to easily try this out on the x86!
 * Read the 'readme' for details..
 */
.data

msg:
.ascii "Hello, ARM World!\n"
len = . - msg

.text

.globl _start
_start:
 /* syscall to issue: write(1, msg, 13)
	  ARM-32 ABI:
	  r7 : syscall #
	   [fyi, the syscall #s can be found here in the kernel src tree:
	   arch/arm/include/uapi/asm/unistd.h ]
	  The first 4 parameters are placed into these registers
	  in this order:
	  r0, r1, r2, r3
	  >4 params => stack is used (=> we'll have to setup the stack prior
	  to invoking the syscall!)
   */
mov %r0, $1   @  ARM: mov <dest>,<src>
              @ put literal '1' in register r0; rep 'stdout' fd
ldr %r1, =msg 
ldr %r2, =len 
mov %r7, $4 
swi $0        @ issue software interrupt, do the syscall

/* exit syscall */
mov %r0, $0 
mov %r7, $1 
swi $0
