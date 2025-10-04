#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/limits.h>

#include <path.h>
#include <safe_mallocs.h>

char **path_entries;

char **tokenize(char *input, char **output, char delim) {

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

	output = safe_realloc(output, output_size);

	ret = strtok(input, delim_string);

	for (size_t i = 0; ret != NULL; i++) {
		output[i] = ret;
		ret = strtok(NULL, delim_string);
	}

	output[delim_count + 1] = NULL;
	return output;
}

char **init_path(char **output) {

	char *path_envvar;
	size_t path_envvar_size;
	char *s;

	path_envvar = getenv("PATH");

	if (path_envvar == NULL)
		return NULL;

	path_envvar_size = strlen(path_envvar) + 1;

	s = safe_malloc(path_envvar_size);
	memcpy(s, path_envvar, path_envvar_size);

	return tokenize(s, output, ':');
}

int handle_path(char *input, char *output, char **paths) {

	if (strchr(input, '/') != NULL) {
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