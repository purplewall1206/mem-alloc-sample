#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/ip.h>
#include <errno.h>
#include <sched.h>
#include <ctype.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <time.h>
#include <signal.h>
#include <sys/resource.h>
#include <linux/netfilter.h>


/* Assign to a specific CPU core */
void assign_to_core(int core_id) {
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(core_id, &mask);
	if(sched_setaffinity(getpid(), sizeof(mask), &mask) < 0)
		printf("[-] Error at sched_setaffinity()");
	return;
}

/* Modify process rlimit for RLIMIT_NOFILE */
void modify_rlimit(void) {
	struct rlimit old_lim, lim, new_lim;
	
	if(getrlimit(RLIMIT_NOFILE, &old_lim) != 0)
		printf("[-] Error in getrlimit()");
		
	lim.rlim_cur = old_lim.rlim_max;
	lim.rlim_max = old_lim.rlim_max;

	if(setrlimit(RLIMIT_NOFILE, &lim) == -1)
		printf("[-] Error at setrlimit()");

	return;
}
#define MAX_FDS 1000000
#define DEF_CORE 0

void spray_seq_op_loop(void) {
	// int fds[MAX_FDS] = { 0 };
    int fd = 0;
	int i = 0;
	
	assign_to_core(DEF_CORE);
	
	modify_rlimit();
	unsigned long c = 0;
	while(1) {
		fd = open("/proc/self/stat", O_RDONLY);
        ++c;
		if (fd == -1) {
            printf("total: %ld\n",c);
            perror("ERROR: ");
            return ;
        }
	}
	return;
}

int main()
{
	int i = 0;
    while (i++ < 2) {
        fork();
        
    }
	spray_seq_op_loop();
	while(1);
    // spray_seq_op_loop();
    return 0;
}

// sudo bpftrace -e 'kfunc:single_open { printf("pid:%d:%s allocation\n", pid, comm);}   kfunc:single_release {printf("pid:%d:%s release\n", pid, comm);}'
// sudo bpftrace -e 'kfunc:single_open { @btopen[kstack]=count(); @commopen[comm]=count();}   kfunc:single_release { @btrelease[kstack]=count(); @commrelease[comm]=count();}' -o test.log

// kfunc:single_open
//     struct file * file;
//     int (*)(struct seq_file *, void *) show;
//     void * data;
//     int retval;

// kfunc:single_release
//     struct inode * inode;
//     struct file * file;
//     int retval;


// single thread
// total: 1048569
// ERROR: : Too many open files
// real    0m2.193s
// user    0m0.052s
// sys     0m2.125s

// gcc -g -o spray_seq_op spray_seq_op.c