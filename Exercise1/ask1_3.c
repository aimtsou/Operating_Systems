#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tree.h"
#include "proc-common.h"

#define SLEEP_PROC_SEC 3

/* 1.3. Send & Handle Signals. */

void fork_procs(struct tree_node *node)
{
	int i, status;
	pid_t p[node->nr_children];
	
	printf("%s, PID = %ld: Starting...\n",
		node->name, (long)getpid());
	change_pname(node->name);
	if (node->nr_children > 0) { 
		// printf("%s, PID = %ld: Creating child...\n", node->name, (long)getpid());
		i = 0;
		do {
			p[i] = fork();
			if (p[i] < 0) {		/* Fork failed */
				perror("fork");
				exit(1);
			}
			if (p[i] == 0) {
				fork_procs(node->children+i);
				exit(1);
			}
			printf("%s, PID = %ld: Created child with PID = %ld...\n",
				node->name, (long)getpid(), (long)p[i]);
			i = i + 1;
		} while (i < node->nr_children && p[i] != 0);
	}

	wait_for_ready_children(node->nr_children);
	raise(SIGSTOP);
	printf("%s, PID = %ld: Is awake...\n",
		node->name, (long)getpid());
	if (node->nr_children > 0) {
		i = 0;
		printf("%s, PID = %ld: Waking up my children...\n",
			node->name, (long)getpid());
		do {
			kill (p[i], SIGCONT);
			p[i] = wait(&status);
			explain_wait_status(p[i], status);
			i = i + 1;
		} while (i < node->nr_children && p[i] != 0);
	}
	else if (node->nr_children == 0) {
		printf("%s, PID = %ld: Executing...\n", node->name, (long)getpid());
		sleep(SLEEP_PROC_SEC);
		printf("%s, PID = %ld: Exiting...\n", node->name, (long)getpid());
		exit(1);
	}
}

/* The initial process forks the root of the process tree,
 * waits for the process tree to be completely created.
 * Then takes a photo of it using show_pstree() using
 * wait_for_ready_children() to wait until the first process 
 * raises SIGSTOP. */

int main(int argc, char *argv[])
{
	pid_t pid;
	int status;
	struct tree_node *root;

	if (argc < 2){
		fprintf(stderr, "Usage: %s <tree_file>\n", argv[0]);
		exit(1);
	}
	root = get_tree_from_file(argv[1]);
	pid = fork();			/* Fork root of process tree */
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {
		fork_procs(root);	/* Creating the process tree by recursion */
		exit(1);
	}
	wait_for_ready_children(1);	/* Father */
	show_pstree(pid);		/* Print the process tree root at pid */
	kill(pid, SIGCONT);
	pid = wait(&status);		/* Wait for the root of the process tree to terminate */
	explain_wait_status(pid, status);

	return 0;
}
