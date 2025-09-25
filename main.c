#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <linux/limits.h>

#define READ_LINESIZE 512

char **tokenize(char *input, char** output, char delim) {

	size_t delim_count = 0;	
	size_t output_size;
	char delim_string[2];
	char *ret;

	delim_string[0] = delim;
	delim_string[1] = '\0';

	for (size_t i = 0; input[i] != '\0'; i++) {
		if (input[i] == delim)
			delim_count++;
	}

	output_size = (delim_count + 2) * sizeof(char *);

	output = realloc(output, output_size);

	ret = strtok(input, delim_string);

	for (size_t i = 0; ret != NULL; i++) {
		output[i] = ret;
		ret = strtok(NULL, delim_string);
	}

	output[delim_count + 1] = NULL;
	return output;
}

static inline char **init_path(char** output) {

	char *path_envvar;
	size_t path_envvar_size;
	char *s;

	path_envvar = getenv("PATH");

	if (path_envvar == NULL)
		return NULL;

	path_envvar_size = strlen(path_envvar) + 1;

	s = malloc(path_envvar_size);
	memcpy(s, path_envvar, path_envvar_size);

	return tokenize(s, output, ':');
}

int handle_path(char *input, char *output, char **paths) {

	if (input[0] == '/' || input[0] == '.') {
		strcpy(output, input);
		return 0;
	}

	for (size_t i = 0; paths[i] != NULL; i++) {
		snprintf(output, PATH_MAX, "%s/%s", paths[i], input);
		if (access(output, F_OK) == 0) {
			return 0;
		}
	}

	return -1;
}

int main(int argc, char **argv, char **envp) {

	char *in_buf = malloc(READ_LINESIZE);
	ssize_t in_size;
	char **cargv;
	size_t linesize = READ_LINESIZE;
	char *nothing = NULL;
	char **path_entries;
	int handle_path_ret;
	char cwd[PATH_MAX];
	char to_exec[PATH_MAX];
	pid_t cpid;

	cargv = NULL;
	path_entries = NULL;
	
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

		in_buf[in_size - 1] = '\0';

		cargv = tokenize(in_buf, cargv, ' ');

		// TODO: fix this
		if (strcmp(cargv[0], "cd") == 0) {
			chdir(cargv[1]);
			continue;
		}

		handle_path_ret = handle_path(cargv[0], to_exec, path_entries);

		if (handle_path_ret == -1) {
			fprintf(stderr, "%s: command not found\n", cargv[0]);
			continue;
		}

		cpid = fork();

		if (cpid == 0) {
			execve(to_exec, cargv, envp);

			// if we get here execve has failed
			error(1, errno, "%s", cargv[0]);
		}

		wait(NULL);
	}

	free(in_buf);
	free(cargv);
	free(path_entries);

	return 0;
}
