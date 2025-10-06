#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <bsh.h>
#include <cmd.h>
#include <tokens.h>
#include <tree.h>
#include <path.h>

static sigset_t blocker;

static int dup2_safe(int oldfd, int newfd) {
    int output;

    output = dup2(oldfd, newfd);

    if (output == -1) {
        error(EXIT_FAILURE, errno, "dup2");
    }

    return output;
}

int execute_chain() {
    kill(0, SIGCONT);

    while (wait(NULL) > 0);
    return 0;
}

static int execute_normal(leaf_t *cmd) {
    pid_t cpid;
    int sig;
    char filename[PATH_MAX];

    cmd->argv = tokenize(cmd->command, NULL, ' ');

    if (handle_path(cmd->argv[0], filename, path_entries) == 0) {
        cmd->command = filename;
    } else {
        error(EXIT_SUCCESS, 0, "command not found");
        return -1;
    }

    cpid = fork();
    if (cpid == -1) {
        error(EXIT_FAILURE, errno, "fork");
    }

    if (cpid == 0) {
        dup2_safe(cmd->stdin, 0);
        dup2_safe(cmd->stdout, 1);
        dup2_safe(cmd->stderr, 2);

        // everything is ready, we wait until the parent signals us to proceed
        sigwait(&blocker, &sig);

        // We go.
        execve(cmd->command, cmd->argv, envp_global);

        // execve() has failed.
        error(EXIT_FAILURE, errno, "%s", cmd->argv[0]);
    }

    // don't care about return status of close() right now
    if (cmd->stdin != 0) {
        close(cmd->stdin);
    }

    if (cmd->stdout != 1) {
        close(cmd->stdout);
    }

    if (cmd->stderr != 2) {
        close(cmd->stderr);
    }

    return 0;
}

static int execute_indir(leaf_t *cmd, leaf_t *file) {
    int outfd;

    outfd = open(file->command, O_CREAT|O_WRONLY, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH));

    if (outfd == -1) {
        error(EXIT_SUCCESS, errno, "%s", file->command);
        return -1;
    }

    cmd->stdout = outfd;
    return execute_normal(cmd);
}

static int execute_pipe(leaf_t *cmd) {
    int pipe_ends[2];
    int pipe_ret;
    int ret;

    pipe_ret = pipe(pipe_ends);

    if (pipe_ret == -1) {
        error(EXIT_SUCCESS, errno, "pipe");
        return -1;
    }

    cmd->stdout = pipe_ends[1];
    ret = execute_normal(cmd);

    if (ret != 0) {
        close(pipe_ends[0]);
        close(pipe_ends[1]);
        return ret;
    }

    return pipe_ends[0];
}

int load_tree(tree_t *input) {
    leaf_t *leftmost;
    leaf_t *next_l;
    node_t *next_n;
    node_t *root;
    int execute_ret;
    token_t parent_type;

    root = input->root;
    execute_ret = 0;
    leftmost = next_leaf(root);
    next_l = NULL;
    next_n = NULL;

    if (leftmost == NULL) {
        // The input has to have at least one node, so we assume root == leftmost
        leftmost = (leaf_t *) root;
    }

    if (sigemptyset(&blocker) != 0) {
        error(EXIT_FAILURE, errno, "sigemptyset");
    }

    if (sigaddset(&blocker, SIGCONT) != 0) {
        error(EXIT_FAILURE, errno, "sigaddset");
    }

    // not a great idea to block sigcont, but we do it anyway
    if (sigprocmask(SIG_SETMASK, &blocker, NULL) != 0) {
        error(EXIT_FAILURE, errno, "sigprocmask");
    }

    while (leftmost != NULL) {
        next_n = next_node((node_t *) leftmost);
        next_l = NULL; // otherwise calling next_leaf() causes a segfault

        // if we don't do this, calling next_leaf() may cause a use-after-free
        if (next_n != NULL) {
            parent_type = next_n->type;
            next_l = next_leaf(next_n);
        }

        // done traversing
        if (next_l == NULL) {
            execute_ret = execute_normal(leftmost);
            kill_leaf(leftmost);
            break;
        }

        switch (parent_type) {
        case REDIRECTION:
            execute_ret = execute_indir(leftmost, next_l);
            next_l = next_leaf((node_t *) leftmost);
            break;
        case PIPE:
            leftmost->stdout = execute_pipe(leftmost);
            next_l->stdin = leftmost->stdout;
            execute_ret = leftmost->stdout;
            break;
        case SEMICOLON:
            execute_ret = execute_normal(leftmost);
            execute_chain();
            break;
        default:
            error(EXIT_SUCCESS, 0, "not implemented");
            execute_ret = -1;
            break;
        }

        kill_leaf(leftmost);
        leftmost = next_l;
        if (execute_ret == -1) break;
    }

    if (sigprocmask(SIG_UNBLOCK, &blocker, NULL) != 0) {
        error(EXIT_FAILURE, errno, "sigprocmask");
    }

    return execute_ret;
}