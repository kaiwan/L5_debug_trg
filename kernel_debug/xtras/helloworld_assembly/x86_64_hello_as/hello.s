# ----------------------------------------------------------------------------------------
# Src:
#  http://cs.lmu.edu/~ray/notes/gasexamples/
#
# Writes "Hello, World" to the console using only system calls. Runs on
# x86_64 Linux only.
# To assemble and run:
#
#     gcc -c hello.s && ld hello.o && ./a.out
#
# or
#
#     gcc -nostdlib hello.s && ./a.out
# ----------------------------------------------------------------------------------------
        .global _start

        .text
_start:
        # syscall to issue: write(1, message, 13)
		  # x86_64 ABI: 
		  # %rax : syscall #
		  # The first 6 parameters are placed into these registers
		  # in this order:
		  # %rdi, %rsi, %rdx, %rcx, %r8 and %r9.
		  # >6 params => stack is used (=> we'll have to setup the stack prior
		  # to invoking the syscall!)
        mov     $1, %rax                # system call 1 is write
		                                 # mov <src>, <dest> ; ATT syntax
        mov     $1, %rdi                # file handle 1 is stdout
        mov     $message, %rsi          # address of string to output
        mov     $13, %rdx               # number of bytes
        syscall                         # invoke operating system to do the write
   # ABI: FYI, the return value will be in %rax

        # exit(0)
        mov     $60, %rax               # system call 60 is exit
        xor     %rdi, %rdi              # we want return code 0
        syscall                         # invoke operating system to exit
message:
        .ascii  "Hello, world\n"
