#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/limits.h>

#include <path.h>
#include <safe_mallocs.h>

char **path_entries;

char *skip_quotes(char *input, size_t *index) {
	size_t i;

	if (input == NULL) return NULL;

	if (index != NULL) {
		i = *index;
	} else {
		i = 0;
	}

	if (input[i] == '\'') {
		do {
			i++;
		} while (input[i] != '\0' && input[i] != '\'');
	} else if (input[i] == '"') {
		do {
			i++;
		} while (input[i] != '\0' && input[i] != '"');
	}

	input[i] = '\0';

	if (index != NULL) {
		*index = i;
	}

	return input;
}

char **tokenize(char *input, char **output, char delim) {
	size_t output_size;
    size_t output_idx;
	char *previous;

    output_idx = 0;
    output_size = 16;
    output = safe_realloc(output, output_size * sizeof(char *));
    previous = input;

	for (size_t i = 0; input[i] != '\0'; i++) {
        if (output_idx >= (output_size - 1)) {
            output_size += 16;
            output = safe_realloc(output, output_size * sizeof (char *));
        }

        if (input[i] == '"' || input[i] == '\'') {
			previous = input + i + 1;
            skip_quotes(input, &i);
        } else if (input[i] == delim) {
            input[i] = '\0';
            while (input[i + 1] == delim && input[i + 1] != '\0') {
                i++;
            }

            output[output_idx] = previous;
            output_idx++;

			// otherwise, an entry in argv with only a null byte shows up at the end
            if (input[i+1] != '\0') {
				previous = input + i + 1;
			}
        }
	}

    output[output_idx] = previous;
    output_idx++;
    output[output_idx] = NULL;

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