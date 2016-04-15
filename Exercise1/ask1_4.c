#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include "tree.h"
#include "proc-common.h"

/* 1.4. Parallel computation of an arithmetic expression. */

void fork_procs(struct tree_node *node, int r_fd, int w_fd)
{
	int i, pfd[2];
	pid_t p[node->nr_children];
	double value, v[2];
	
	/* Generating the process tree */
	printf("PID = %ld: Starting...\n",
		(long)getpid());
	change_pname(node->name);
	
	if ((node->nr_children == 2) && ((strcmp(node->name,"*") == 0) || (strcmp(node->name,"+") == 0))) {
		if (pipe(pfd) == -1) {
			perror("pipe");
			exit(1);
		}
		printf ("new pipe read end = %d \n", pfd[0]);
		printf ("new pipe write end = %d \n", pfd[1]);
		for (i = 0; i < node->nr_children; i++) {
			p[i] = fork();
			if (p[i] < 0) {
				perror("fork");
				exit(1);
			}
			if (p[i] == 0) {
				fork_procs(node->children+i, pfd[0], pfd[1]);
				exit(1);
			}
			printf("PID = %ld: Created child with PID = %ld...\n",
				(long)getpid(), (long)p[i]);
		}
		
		for (i = 0; i < node->nr_children; i++) {
			printf ("PID = %ld: Read from pipe, read pfd = %d\n",
				(long)getpid(), pfd[0]);
			if (read(pfd[0], &value, sizeof(value)) != sizeof(value)) {
				perror("read from pipe");
				exit(1);
			}			
			v[i] = value;
			printf("PID = %ld: Received value = %f from my child with PID = %ld\n",
				(long)getpid(), value, (long)p[i]);
		}
		if (strcmp(node->name,"*") == 0)
			value = v[0] * v[1];
		else
			value = v[0] + v[1];
		printf ("PID = %ld: Write to pipe, write pfd = %d\n",
			(long)getpid(), w_fd);
		if (write(w_fd, &value, sizeof(value)) != sizeof(value)) {
			perror("write to pipe");
			exit(1);
		}
		printf("PID = %ld: Send to my father the value = %f\n",
			(long)getpid(), value);
		exit(1);
	}
	/* Leaf process */
	else if (node->nr_children == 0) {
		value = atoi(node->name);
		printf ("PID = %ld: Write to pipe, write pfd = %d\n",
			(long)getpid(), w_fd);
		if (write(w_fd, &value, sizeof(value)) != sizeof(value)) {
			perror("write to pipe");
			exit(1);
		}
		printf("PID = %ld: Send to my father the value = %f\n",
			(long)getpid(), value);
		exit(1);
	}
	else {
		printf("Tree format error...\n");
		exit(0);
	}
}

int main(int argc, char *argv[])
{
	pid_t pid;
	int pfd[2];
	double value;
	struct tree_node *root;

	/* Check if there is an input file */
	if (argc < 2){
		printf("Usage: %s <tree_file>\n", argv[0]);
		exit(1);
	}
	root = get_tree_from_file(argv[1]);	
	/* Create the pipe between main and tree root */
	if (pipe(pfd) < 0) {
		perror("pipe");
		exit(1);
	}
	printf ("pipe read end = %d \n", pfd[0]);
	printf ("pipe write end = %d \n", pfd[1]);
	/* Fork root of process tree */
	pid = fork();			
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid == 0) {
		/* Call the function to create the process tree */
		fork_procs(root, pfd[0], pfd[1]);	
		exit(1);
	}
	/* Print the process tree root at pid */
	// show_pstree(pid);
	/* Wait for read the final value */
	if (read(pfd[0], &value, sizeof(value)) != sizeof(value)) {
		perror("read from pipe");
		exit(1);
	}
	printf("Initial process, PID = %ld received value: value = %f\n",
		(long)getpid(), value);
	
	exit(0);			
	
	return 0;
}
