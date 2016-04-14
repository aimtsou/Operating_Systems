#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

/* 1.1. Create specific process tree. */

void fork_procs()
{
	pid_t p1, p2, p3;
	int status1, status2, status3;
	
	printf("A, PID = %ld: Starting...\n", (long)getpid());
	change_pname("A");			/* initial process is A */
	printf("A, PID = %ld: Creating child...\n",
		(long)getpid());
	p1 = fork();
	if (p1 != 0)
		p2 = fork();
	if (p1 < 0) {				/* fork failed */
		perror("fork");
		exit(1);
	}
	if (p1 == 0) {
		printf("B, PID = %ld: Starting...\n", (long)getpid());
		change_pname("B");		/* B is the 1st child of process A */
		printf("B, PID = %ld: Creating child...\n",
			(long)getpid());
		p3 = fork();
		if (p3 < 0) {			/* fork failed */
			perror("fork");
			exit(1);
		}
		if (p3 == 0) {
			printf("D, PID = %ld: Starting...\n", (long)getpid());
			change_pname("D");	/* D is the child of process B */
			printf("D, PID = %ld: Sleeping...\n", (long)getpid());
			sleep(SLEEP_PROC_SEC);
			printf("D, PID = %ld: Exiting...\n", (long)getpid());
			exit(13);
		}
		printf("B, PID = %ld: Created child with PID_D = %ld, waiting for it to terminate...\n",
			(long)getpid(), (long)p3);
		p3 = wait(&status3);
		explain_wait_status(p3, status3);
		printf("B, PID = %ld: Exiting...\n", (long)getpid());
		exit(19);
	}
	if (p2 < 0) {				/* fork failed */
		perror("fork");
		exit(1);
	}
	if (p2 == 0) {				/* C is the 2nd child of process A */
		printf("C, PID = %ld: Starting...\n", (long)getpid());
		change_pname("C");
		printf("C, PID = %ld: Sleeping...\n", (long)getpid());
		sleep(SLEEP_PROC_SEC);
		printf("C, PID = %ld: Exiting...\n", (long)getpid());
		exit(17);
	}
	printf("A, PID = %ld: Created children with PID_B = %ld and PID_C = %ld waiting for them to terminate...\n",
		(long)getpid(), (long)p1, (long)p2);
	p1 = wait(&status1);
	explain_wait_status(p1, status1);
	if (p2 != 0) {
		p2 = wait(&status2);
		explain_wait_status(p2, status2);
	}
	printf("A, PID = %ld: Exiting...\n", (long)getpid());
	exit(16);
}

/* The initial process forks the root of the process tree,
 * waits for the process tree to be completely created,
 * Then takes a photo of it using show_pstree() waiting
 * for a few seconds for the process tree to be ready. */

int main(void)
{
	pid_t pid;
	int status;

	pid = fork();				/* Fork root of process tree */
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {
		fork_procs();			/* process A */
		exit(1);
	}
	
	sleep(SLEEP_TREE_SEC);			/* root of process tree */
	show_pstree(pid);
	pid = wait(&status);
	explain_wait_status(pid, status);
	
	return 0;
}
