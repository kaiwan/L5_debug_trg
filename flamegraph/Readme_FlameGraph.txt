CPU FlameGraphs are a visualization of profiled software, allowing the most
frequent code-paths to be identified quickly and accurately. They can be
generated using Brendan Gregg's open source programs on
github.com/brendangregg/FlameGraph

For the developer's convenience, we provide a wrapper to generate CPU FlameGraphs.
Run the flame_grapher.sh script, follow instructions.
The result is a rendered SVG file - the FlameGraph!. 
You can view it in a web browser.

The *style* of the flamegraph can be one of:
- regular: 'upward-growing', or
- icicle : 'downward-growing' [default]
This script keeps icicle-style as the default as it's technically correct; on
pretty much all modern arch's (CPUs) stacks grown towards lower (virual) addresses!
You can change it by changing:
flame_grapher.sh:STYLE_INVERTED_ICICLE=1  to the value 0.

=========
IMP NOTE
=========
Getting a decent FlameGraph REQUIRES:
- frame pointers (-fomit-frame-pointer is the typical GCC flag!)
  - possible exception case is the Linux kernel itself; it has intelligent
    algorithms to emit an accurate stack trace even in the absence of frame pointers
  - symbols (can use a separate symbol file)


==================
Example sessions:
==================

$ ./flame_grapher.sh 
Usage: flame_grapher.sh [process-PID-to-sample] svg-out-filename (without .svg)
 No PID implies the entire system is sampled...
flame_grapher.sh --help , to show this help screen.
$ 


1. Capturing a FlameGraph of a system-wide trace
------------------------------------------------
$ ./flame_grapher.sh sys1
### flame_grapher.sh: recording samples system-wide now...Press ^C to stop...
^C[ perf record: Woken up 120 times to write data ]
[ perf record: Captured and wrote 50.111 MB perf.data (6537 samples) ]

2flameg.sh: Working ... generating SVG file "/tmp/flamegraphs/sys1/sys1.svg"...
addr2line: DWARF error: section .debug_info is larger than its filesize! (0x93ef57 vs 0x530ea0)

[...]
-rw-rw-r-- 1 kaiwan kaiwan 2.3M Jun 23 18:09 /tmp/flamegraphs/sys1/sys1.svg

View the above SVG file in your web browser to see and zoom into the CPU FlameGraph.
$ 


2. Capturing a FlameGraph of a particular process trace (VirtualBox)
--------------------------------------------------------------------
$ ./flame_grapher.sh -p 991798 -o vbox1
### flame_grapher.sh: recording samples on process PID 991798 now... Press ^C to stop...
^C[ perf record: Woken up 14 times to write data ]
[ perf record: Captured and wrote 5.476 MB perf.data (644 samples) ]

2flameg.sh: Working ... generating SVG file "/tmp/flamegraphs/vbox1/vbox1.svg"...
addr2line: DWARF error: section .debug_info is larger than its filesize! (0x93ef57 vs 0x530ea0)
[...]
-rw-rw-r-- 1 kaiwan kaiwan 264K Jun 23 18:18 /tmp/flamegraphs/vbox1/vbox1.svg

View the above SVG file in your web browser to see and zoom into the CPU FlameGraph.
$ 
