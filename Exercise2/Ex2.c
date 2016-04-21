#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "proc-common.h"
#include "tree.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

void fork_procs(struct tree_node *node) {
	int i, status;
	pid_t pid[node->number];
	
	printf("%s, PID = %ld: Starting...\n", node->name, (long)getpid());
	change_pname(node->name);
	if (node->number == 0) {		
		printf("%s, PID = %ld: Sleeping...\n", node->name, (long)getpid());
		sleep(SLEEP_PROC_SEC);
		printf("%s, PID = %ld: Exiting...\n", node->name, (long)getpid());
		exit(1);
	}
	else if (node->number > 0) {
		printf("%s, PID = %ld: Creating CHild...\n", node->name, (long)getpid());
		i=0;
		while (i < pid[number] && pid[i] != 0) {
		`	pid[i] = fork();
			if (pid < 0) {
				perror("main: fork");
				exit(1);
			}
			if (p[i] == 0) {
				fork_procs(node->children+i);
				exit(1);
			}
			printf("%s, PID = %ld: Created child with PID = %ld, waiting for it to terminate...\n", node->name, (long)getpid(), (long)p[i]);
			i++;
		}
		i=0;
		while (i < pid[number] && pid[i] != 0) {
			pid[i] = wait(&status);
			explain_wait_status(pid[i], status);
			i++;
		}
		printf("%s, PID = %ld: Exiting...\n", node->name, (long)getpid());
		exit(1);
	}
}

int main(int argc, char **argv) {
	pid_t pid;
	int status;
	struct tree_node *root;
	
	if (argc != 2) {
		printf(stderr, "Usage: %s <input_tree_file>\n\n", argv[0]);
		exit(1);
	}

	root = get_tree_from_file(argv[1]);
	
	pid = fork();
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	
	if (pid == 0) {
		fork_procs(root);
		exit(1);
	}
	
	sleep(SLEEP_TREE_SEC);
	show_pstree(pid);
	
	pid = wait(&status);
	explain_wait_status(pid, status);
	
	return 0;
}