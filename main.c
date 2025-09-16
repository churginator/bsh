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

char *const *tokenize(char *input, ssize_t inlen) {

	size_t space_count = 0;
	
	for (ssize_t i = 0; i < inlen; i++) {
		if (input[i] == ' ')
			space_count++;
	}

	char **output = malloc((space_count + 2) * sizeof(char *));
	output[0] = strtok(input, " ");

	for (size_t i = 1; i <= space_count; i++) {
		output[i] = strtok(NULL, " ");
	}

	output[space_count + 1] = NULL;

	return output;
}

int main(int argc, char **argv) {

	char *in_buf = malloc(READ_LINESIZE);
	ssize_t in_size;
	char *const *cargv;
	size_t linesize = READ_LINESIZE;
	char *nothing = NULL;
	pid_t cpid;

	while (1) {
		char cwd[PATH_MAX];
		if (getcwd(cwd, sizeof(cwd)) == NULL)
			error(EXIT_SUCCESS, errno, "getcwd");

		fputs("> ", stdout);
		in_size = getline(&in_buf, &linesize, stdin);

		if (in_size == -1) {
			fflush(stdout);
			exit(0);
		} else if (in_size == 1) {
			continue;
		}

		in_buf[in_size - 1] = '\0';

		cpid = fork();

		if (cpid == 0) {
			char *s = malloc(in_size + 1);
			if (s == NULL)
				error(1, errno, "malloc");

			memcpy(s, in_buf, in_size + 1);
			cargv = tokenize(s, in_size);

			execve(cargv[0], cargv, &nothing);

			// if we get here execve has failed
			error(1, errno, "%s", in_buf);
		}

		wait(NULL);
	}

	free(in_buf);
	return 0;
}
