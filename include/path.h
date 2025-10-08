#ifndef PATH_H
#define PATH_H

char *skip_quotes(char *input, size_t *index);
char **tokenize(char *input, char **output, char delim);
char **init_path(char **output);
int handle_path(char *input, char *output, char **paths);
extern char **path_entries;

#endif // PATH_H