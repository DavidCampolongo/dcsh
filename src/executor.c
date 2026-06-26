#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "dcsh.h"
#include "executor.h"

static int is_pending_operator(const char *token)
{
    return strcmp(token, "|") == 0 ||
           strcmp(token, "<") == 0 ||
           strcmp(token, ">") == 0;
}

static int contains_pending_operator(const TokenList *tokens)
{
    for (int i = 0; i < tokens->count; i++) {
        if (is_pending_operator(tokens->items[i])) {
            return 1;
        }
    }

    return 0;
}

static void copy_tokens_to_argv(const TokenList *tokens, char *argv[])
{
    for (int i = 0; i < tokens->count; i++) {
        argv[i] = tokens->items[i];
    }

    argv[tokens->count] = NULL;
}

static int wait_for_child(pid_t child_pid)
{
    int status;

    while (waitpid(child_pid, &status, 0) == -1) {
        if (errno == EINTR) {
            continue;
        }

        perror("dcsh: waitpid");
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }

    return -1;
}

int execute_external_command(const TokenList *tokens)
{
    char *argv[DCSH_TOKEN_MAX + 1];
    pid_t child_pid;

    if (tokens->count == 0) {
        return 0;
    }

    if (contains_pending_operator(tokens)) {
        fprintf(stderr, "dcsh: pipes and redirection are not implemented yet\n");
        return -1;
    }

    copy_tokens_to_argv(tokens, argv);

    child_pid = fork();

    if (child_pid == -1) {
        perror("dcsh: fork");
        return -1;
    }

    if (child_pid == 0) {
        execvp(argv[0], argv);

        fprintf(stderr, "dcsh: %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }

    return wait_for_child(child_pid);
}