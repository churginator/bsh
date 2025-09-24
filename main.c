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

char **tokenize(char *input, char** output) {

	size_t space_count = 0;	
	size_t input_size;
	size_t output_size;
	char *ret;

	input_size = strlen(input) + 1;

	for (size_t i = 0; i < input_size; i++) {
		if (input[i] == ' ')
			space_count++;
	}

	output_size = (space_count + 2) * sizeof(char *);

	output = realloc(output, output_size);

	ret = strtok(input, " ");

	for (size_t i = 0; ret != NULL; i++) {
		output[i] = ret;
		ret = strtok(NULL, " ");
	}

	output[space_count + 1] = NULL;
	return output;
}

void handle_path(char **input, char *output) {
	
	if (access(input[0], F_OK) != 0) {
		strcpy(output, "/bin/");
		strcpy(output + strlen("/bin/"), input[0]);
	} else {
		strcpy(output, input[0]);
	}
}

int main(int argc, char **argv) {

	char *in_buf = malloc(READ_LINESIZE);
	ssize_t in_size;
	char **cargv;
	size_t linesize = READ_LINESIZE;
	char *nothing = NULL;
	char cwd[PATH_MAX];
	char to_exec[PATH_MAX];
	pid_t cpid;

	cargv = NULL;

	while (1) {
		if (getcwd(cwd, sizeof(cwd)) == NULL)
			error(EXIT_SUCCESS, errno, "getcwd");

		fputs(cwd, stdout);
		fputs("$ ", stdout);
		in_size = getline(&in_buf, &linesize, stdin);

		if (in_size == -1) {
			fputs("\nexit\n", stdout);
			exit(0);
		} else if (in_size == 1) {
			continue;
		}

		in_buf[in_size - 1] = '\0';

		cargv = tokenize(in_buf, cargv);

		if (strcmp(cargv[0], "cd") == 0) {
			chdir(cargv[1]);
			continue;
		}

		handle_path(cargv, to_exec);

		cpid = fork();

		if (cpid == 0) {
			execve(to_exec, cargv, &nothing);

			// if we get here execve has failed
			error(1, errno, "%s", cargv[0]);
		}

		wait(NULL);
	}

	free(in_buf);
	free(cargv);

	return 0;
}
