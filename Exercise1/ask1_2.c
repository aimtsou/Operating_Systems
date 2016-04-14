#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"
#include "tree.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

/* 1.2. Create an arbitrary process tree. */

void fork_procs(struct tree_node *node)
{
	int i, status;
	pid_t p[node->nr_children];

	printf("%s, PID = %ld: Starting...\n",
		node->name, (long)getpid());
	change_pname(node->name);
	if (node->nr_children == 0) {		/* Check how many children a process has. */
		printf("%s, PID = %ld: Sleeping...\n", node->name, (long)getpid());
		sleep(SLEEP_PROC_SEC);
		printf("%s, PID = %ld: Exiting...\n", node->name, (long)getpid());
		exit(1);
	}
	else if (node->nr_children > 0) {	/* More than one child */
		printf("%s, PID = %ld: Creating child...\n", node->name, (long)getpid());
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
			printf("%s, PID = %ld: Created child with PID = %ld, waiting for it to terminate...\n",
				node->name, (long)getpid(), (long)p[i]);
			i = i + 1;
		} while (i < node->nr_children && p[i] != 0);
		i = 0;
		do {
			p[i] = wait(&status);
			explain_wait_status(p[i], status);
			i = i + 1;
		} while (i < node->nr_children && p[i] != 0);
		printf("%s, PID = %ld: Exiting...\n", node->name, (long)getpid());
		exit(1);
	}
}

/* The initial process forks the root of the process tree,
 * waits for the process tree to be completely created,
 * Then takes a photo of it using show_pstree() waiting
 * for a few seconds for the process tree to be ready. */

int main(int argc, char *argv[])
{
	pid_t pid;
	int status;
	struct tree_node *root;

	if (argc != 2) {
		printf("Usage: %s <input_tree_file>\n\n", argv[0]);
		exit(1);
	}
	root = get_tree_from_file(argv[1]);
	pid = fork();			/* Fork root of process tree */
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {			/* Creating the process tree by recursion */
		fork_procs(root);
		exit(1);
	}
	sleep(SLEEP_TREE_SEC);
	show_pstree(pid);
	pid = wait(&status);
	explain_wait_status(pid, status);

	return 0;
}
