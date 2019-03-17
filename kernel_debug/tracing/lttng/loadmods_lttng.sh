
MODS="lttng-clock                            lttng-probe-net                          lttng-probe-v4l2
lttng-clock-plugin-test                  lttng-probe-power                        lttng-probe-vmscan
lttng-kprobes                            lttng-probe-printk                       lttng-probe-workqueue
lttng-kretprobes                         lttng-probe-random                       lttng-probe-writeback
lttng-lib-ring-buffer                    lttng-probe-rcu                          lttng-ring-buffer-client-discard
lttng-probe-asoc                         lttng-probe-regulator                    lttng-ring-buffer-client-mmap-discard
lttng-probe-block                        lttng-probe-sched                        lttng-ring-buffer-client-mmap-overwrite
lttng-probe-compaction                   lttng-probe-scsi                         lttng-ring-buffer-client-overwrite
lttng-probe-gpio                         lttng-probe-signal                       lttng-ring-buffer-metadata-client
lttng-probe-irq                          lttng-probe-skb                          lttng-ring-buffer-metadata-mmap-client
lttng-probe-jbd2                         lttng-probe-sock                         lttng-statedump
lttng-probe-kmem                         lttng-probe-statedump                    lttng-test
lttng-probe-kvm                          lttng-probe-sunrpc                       lttng-tracer
lttng-probe-module                       lttng-probe-timer                        
lttng-probe-napi                         lttng-probe-udp"

for mod in ${MODS}
do
  echo "kmod: ${mod}"
  sudo modprobe ${mod}
done
