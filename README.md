# mem-alloc-sample

## apache: phoronix benchmark result + bpftrace sampling
- sample-apache.(pdf|cvs): apache benchmark
- sample.log: bpftrace sample log

```
sudo bpftrace -e 'tracepoint:kmem:kmalloc { @[kstack]=count();}' -o sample-apache.log
phoronix benchmark apache: 7-test all options
```

executing all benchmark under phoronix-apache, it seems `sk_alloc` eat 2.5TB memory in total.


```
@[
    __kmalloc_track_caller+392
    __kmalloc_track_caller+392
    krealloc+84
    nf_ct_ext_add+157
    init_conntrack.constprop.0+491
    resolve_normal_ct+441
    nf_conntrack_in+219
    ipv4_conntrack_local+72
    nf_hook_slow+65
    __ip_local_out+219
    ip_local_out+29
    __ip_queue_xmit+379
    ip_queue_xmit+21
    __tcp_transmit_skb+2237
    tcp_connect+1009
    tcp_v4_connect+951
    __inet_stream_connect+206
    inet_stream_connect+59
    __sys_connect_file+95
    __sys_connect+162
    __x64_sys_connect+24
    do_syscall_64+97
    entry_SYSCALL_64_after_hwframe+68
]: 664959747
@[
    __kmalloc+391
    __kmalloc+391
    security_sk_alloc+121
    sk_prot_alloc+116
    sk_alloc+49
    inet_create.part.0.constprop.0+192
    inet_create+28
    __sock_create+267
    __sys_socket+93
    __x64_sys_socket+23
    do_syscall_64+97
    entry_SYSCALL_64_after_hwframe+68
]: 664959968
```

```
python3 -c 'print((664959747*4096)/(2**30))'
2536.620128631592

python3 -c 'print((664959968*4096)/(2**30))'
2536.6209716796875
```


## spray: seq_operation spray + bpftrace sampling

- spray_seq_op.c: fork 32 process to spray `seq_operations`, each process spray 1048569 files at most.
- sample.log: bpftrace allocating log(not accurate! OOM killer kills everything)
- dmesg.log: OOM killer log

4000MB memory VM, memory exhausts in few seconds.

```
sudo bpftrace -e 'kfunc:single_open { @btopen[kstack]=count(); @commopen[comm]=count();}   kfunc:single_release { @btrelease[kstack]=count(); @commrelease[comm]=count();}' -o test.log
```


```log
Out of memory: Killed process 2765 (node) total-vm:835420kB, anon-rss:0kB, file-rss:0kB, shmem-rss:0kB, UID:1000 pgtables:672kB oom_score_adj:0
[  804.478288] oom_reaper: reaped process 2765 (node), now anon-rss:0kB, file-rss:0kB, shmem-rss:0kB
[  804.721116] Out of memory: Killed process 636 (networkd-dispat) total-vm:42268kB, anon-rss:0kB, file-rss:0kB, shmem-rss:0kB, UID:0 pgtables:116kB oom_score_adj:0
[  804.722260] oom_reaper: reaped process 636 (networkd-dispat), now anon-rss:0kB, file-rss:0kB, shmem-rss:0kB
[  804.738492] Out of memory: Killed process 744 (unattended-upgr) total-vm:119592kB, anon-rss:0kB, file-rss:0kB, shmem-rss:0kB, UID:0 pgtables:128kB oom_score_adj:0
[  804.738892] oom_reaper: reaped process 744 (unattended-upgr), now anon-rss:0kB, file-rss:0kB, shmem-rss:0kB
[  804.767114] Out of memory: Killed process 1612 (goa-daemon) total-vm:560256kB, anon-rss:0kB, file-rss:0kB, shmem-rss:0kB, UID:1000 pgtables:260kB oom_score_adj:0

```