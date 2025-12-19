#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <linux/limits.h>
#include <stdbool.h>

#include <lexer.h>
#include <safe_mallocs.h>
#include <tree.h>
#include <cmd.h>
#include <path.h>

char **envp_global;
bool interactive;

int main(int argc, char **argv, char **envp)
{
	char *in_buf;
	FILE *source;
	ssize_t in_size;
	tree_t *to_exec;
	size_t linesize;
	char cwd[PATH_MAX];

	source = stdin;
	interactive = true;
	linesize = 512;
	in_buf = NULL;
	path_entries = NULL;
	envp_global = envp;

	path_entries = init_path(path_entries);

	if (argc > 1) {
		interactive = false;
		source = fopen(argv[1], "r");
		if (source == NULL) {
			error(EXIT_FAILURE, errno, "%s", argv[1]);
		}
	}

	while (1) {
		if (getcwd(cwd, sizeof(cwd)) == NULL)
			error(EXIT_SUCCESS, errno, "getcwd");

		if (interactive) {
			fputs(cwd, stdout);
			fputs("$ ", stdout);
		}

		in_size = getline(&in_buf, &linesize, source);

		if (in_size == -1) {
			if (interactive)
				putchar('\n');
			exit(0);
		} else if (in_size == 1) {
			continue;
		}

		in_buf[in_size - 1] = '\0'; // newline begone
		to_exec = lex(in_buf);

		load_tree(to_exec);
		execute_chain(); // nothing happens if load_tree() failed

		free(to_exec); // loading the tree should free all the nodes
	}

	fclose(source);
	free(in_buf);
	free(path_entries);

	return 0;
}