#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <error.h>
#include <errno.h>

#define READ_LINESIZE 512

int main(int argc, char **argv) {

	char *in_buf = malloc(READ_LINESIZE);
	size_t linesize = READ_LINESIZE;
	char *nothing = NULL;
	size_t in_size;
	pid_t cpid;

	while (1) {
		fputs("> ", stdout);
		in_size = getline(&in_buf, &linesize, stdin);
		in_buf[in_size - 1] = '\0';

		cpid = fork();

		if (cpid == 0) {
			int ret = execve(in_buf, &nothing, &nothing);
			if (ret) {
				error(0, errno, "%s", in_buf);
			}
		}

		wait(NULL);
	}

	return 0;
}
