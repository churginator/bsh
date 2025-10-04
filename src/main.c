#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <linux/limits.h>

#include <lexer.h>
#include <safe_mallocs.h>
#include <tree.h>
#include <cmd.h>
#include <path.h>

char **envp_global;

int main(int argc, char **argv, char **envp)
{
	char *in_buf;
	ssize_t in_size;
	tree_t *to_exec;
	size_t linesize;
	char cwd[PATH_MAX];

	linesize = 512;
	in_buf = NULL;
	path_entries = NULL;
	envp_global = envp;

	path_entries = init_path(path_entries);

	while (1) {
		if (getcwd(cwd, sizeof(cwd)) == NULL)
			error(EXIT_SUCCESS, errno, "getcwd");

		fputs(cwd, stdout);
		fputs("$ ", stdout);
		in_size = getline(&in_buf, &linesize, stdin);

		if (in_size == -1) {
			putchar('\n');
			exit(0);
		} else if (in_size == 1) {
			continue;
		}

		in_buf[in_size - 1] = '\0'; // newline begone
		to_exec = lex(in_buf);
		if (load_tree(to_exec) == 0) {
			execute_chain();
		}

		free(to_exec); // loading the tree should free all the nodes
	}

	free(in_buf);
	free(path_entries);

	return 0;
}