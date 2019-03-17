Ref: [1]

NOTE- Tested on Ubuntu 17.10 only.

Can run an ARM (Aarch32) binary on x86 Linux!
How?? Works on x86 because of QEMU ARM emulation!
Works once qemu-arm-static pkg is installed.

Can even single-step through the code w/ gdb-multiarch ! (see [1]).

Q. How does an ARM binary executable get executed on x86 Linux?
A. It's due to QEMU ARM usermode emulation !

From [2]: 
"... user mode emulation is a much better fit. In that mode, QEMU will run the
binary code of a foreign architecture as a host process, and at the same time
translate any guest system calls to host system calls."

If you check internally (i used strace), the key syscall seems to be 'arch_prctl()':

$ strace qemu-arm-static ./helloarm
execve("/usr/bin/qemu-arm-static", ["qemu-arm-static", "./helloarm"], [/* 75 vars */]) = 0
uname({sysname="Linux", nodename="kaiwan-T460", ...}) = 0
brk(NULL)                               = 0x62a70000
brk(0x62a71240)                         = 0x62a71240
arch_prctl(ARCH_SET_FS, 0x62a70900)     = 0
set_tid_address(0x62a70bd0)             = 2918
...
$ 

So, once the qemu-system-arm package is installed, all these usermode cpu
emulators are available:

$ qemu-<tab><tab>
qemu-aarch64-static       qemu-make-debian-root     qemu-or32-static          qemu-sparc-static
qemu-alpha-static         qemu-microblazeel-static  qemu-ppc64abi32-static    qemu-system-aarch64
qemu-armeb-static         qemu-microblaze-static    qemu-ppc64le-static       qemu-system-arm
qemu-arm-static           qemu-mips64el-static      qemu-ppc64-static         qemu-system-i386
qemu-cris-static          qemu-mips64-static        qemu-ppc-static           qemu-system-x86_64
qemu-debootstrap          qemu-mipsel-static        qemu-s390x-static         qemu-system-x86_64-spice
qemu-i386-static          qemu-mipsn32el-static     qemu-sh4eb-static         qemu-tilegx-static
qemu-img                  qemu-mipsn32-static       qemu-sh4-static           qemu-x86_64-static
qemu-io                   qemu-mips-static          qemu-sparc32plus-static   
qemu-m68k-static          qemu-nbd                  qemu-sparc64-static       
$
[Ignore the qemu-system-foo - they're the _system_ emulators, i.e., they emulate a
full-fledged guest VM; see the 'qemu-foo-static' -> user emulation of cpu 'foo'!].

Also, the interpretation of a different binary format can be done via the
'binfmt_misc' filesystem.
From [2]:
"Let's think about how binfmt_misc works for a moment. Some process calls
execve() with a filename containing an ARM executable, the kernel tries
to load it with its standard binary format handlers, i.e native ELF and
shebang scripts (yes this is handled in the kernel), and if they fail tries
to load it with binfmt_misc. Then, binfmt_misc matches the executable
signature with the one registered to run with  /usr/bin/qemu-arm-static,
and then creates a new exec request to the kernel, this time requesting
to run the interpreter, passing the original ARM executable as a
parameter. ..."

$ mount|grep "^binfmt_misc"
binfmt_misc on /proc/sys/fs/binfmt_misc type binfmt_misc (rw,relatime)
$ cat /proc/sys/fs/binfmt_misc/qemu-arm
enabled
interpreter /usr/bin/qemu-arm-static
flags: OC
offset 0
magic 7f454c4601010100000000000000000002002800
mask ffffffffffffff00fffffffffffffffffeffffff
$

For arch_prctl(2), can setarch(1) be used?
No, it only supports x86-based stuff..
 (viz., uname26 linux32 linux64 i386 i486 i586 i686 athlon x86_64)

Cool stuff!

Ref: 
[1] http://ubuntuforums.org/showthread.php?t=2010979
[2] https://resin.io/blog/building-arm-containers-on-any-x86-machine-even-dockerhub/
