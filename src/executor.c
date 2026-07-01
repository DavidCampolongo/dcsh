#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "dcsh.h"
#include "executor.h"

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

static int parse_command(const TokenList *tokens,
                         char *argv[],
                         const char **input_path,
                         const char **output_path)
{
    int argc = 0;

    *input_path = NULL;
    *output_path = NULL;

    for (int i = 0; i < tokens->count; i++) {
        char *token = tokens->items[i];

        if (strcmp(token, "|") == 0) {
            fprintf(stderr, "dcsh: pipes are not implemented yet\n");
            return -1;
        }

        if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0) {
            const int is_input = strcmp(token, "<") == 0;

            if (i + 1 >= tokens->count ||
                strcmp(tokens->items[i + 1], "|") == 0 ||
                strcmp(tokens->items[i + 1], "<") == 0 ||
                strcmp(tokens->items[i + 1], ">") == 0) {
                fprintf(stderr, "dcsh: expected file after '%s'\n", token);
                return -1;
            }

            if (is_input) {
                if (*input_path != NULL) {
                    fprintf(stderr, "dcsh: multiple input redirects\n");
                    return -1;
                }

                *input_path = tokens->items[i + 1];
            } else {
                if (*output_path != NULL) {
                    fprintf(stderr, "dcsh: multiple output redirects\n");
                    return -1;
                }

                *output_path = tokens->items[i + 1];
            }

            i++;
            continue;
        }

        argv[argc] = token;
        argc++;
    }

    argv[argc] = NULL;

    if (argc == 0) {
        fprintf(stderr, "dcsh: missing command\n");
        return -1;
    }

    return 0;
}

static int redirect_fd(const char *path, int open_flags, mode_t mode, int target_fd)
{
    int fd = open(path, open_flags, mode);

    if (fd == -1) {
        fprintf(stderr, "dcsh: %s: %s\n", path, strerror(errno));
        return -1;
    }

    if (dup2(fd, target_fd) == -1) {
        fprintf(stderr, "dcsh: dup2: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    if (close(fd) == -1) {
        fprintf(stderr, "dcsh: close: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

static int apply_redirections(const char *input_path, const char *output_path)
{
    if (input_path != NULL &&
        redirect_fd(input_path, O_RDONLY, 0, STDIN_FILENO) != 0) {
        return -1;
    }

    if (output_path != NULL &&
        redirect_fd(output_path,
                    O_WRONLY | O_CREAT | O_TRUNC,
                    0644,
                    STDOUT_FILENO) != 0) {
        return -1;
    }

    return 0;
}

static int find_pipe_index(const TokenList *tokens)
{
    int pipe_index = -1;

    for (int i = 0; i < tokens->count; i++) {
        if (strcmp(tokens->items[i], "|") == 0) {
            if (pipe_index != -1) {
                fprintf(stderr, "dcsh: multiple pipes are not implemented yet\n");
                return -2;
            }

            pipe_index = i;
        }
    }

    return pipe_index;
}

static void copy_token_range(const TokenList *tokens,
                             int start,
                             int end,
                             TokenList *command)
{
    command->count = 0;

    for (int i = start; i < end; i++) {
        command->items[command->count] = tokens->items[i];
        command->count++;
    }
}

static void close_pipe_fds(int pipe_fds[2])
{
    close(pipe_fds[0]);
    close(pipe_fds[1]);
}

static void run_child_command(char *argv[],
                              const char *input_path,
                              const char *output_path)
{
    if (apply_redirections(input_path, output_path) != 0) {
        _exit(1);
    }

    execvp(argv[0], argv);

    fprintf(stderr, "dcsh: %s: %s\n", argv[0], strerror(errno));
    _exit(127);
}

static int execute_pipeline(const TokenList *tokens, int pipe_index)
{
    TokenList left_tokens;
    TokenList right_tokens;
    char *left_argv[DCSH_TOKEN_MAX + 1];
    char *right_argv[DCSH_TOKEN_MAX + 1];
    const char *left_input_path;
    const char *left_output_path;
    const char *right_input_path;
    const char *right_output_path;
    int pipe_fds[2];
    pid_t left_pid;
    pid_t right_pid;
    int left_status;
    int right_status;

    if (pipe_index == 0 || pipe_index == tokens->count - 1) {
        fprintf(stderr, "dcsh: expected command on both sides of pipe\n");
        return -1;
    }

    copy_token_range(tokens, 0, pipe_index, &left_tokens);
    copy_token_range(tokens, pipe_index + 1, tokens->count, &right_tokens);

    if (parse_command(&left_tokens,
                      left_argv,
                      &left_input_path,
                      &left_output_path) != 0 ||
        parse_command(&right_tokens,
                      right_argv,
                      &right_input_path,
                      &right_output_path) != 0) {
        return -1;
    }

    if (pipe(pipe_fds) == -1) {
        perror("dcsh: pipe");
        return -1;
    }

    left_pid = fork();

    if (left_pid == -1) {
        perror("dcsh: fork");
        close_pipe_fds(pipe_fds);
        return -1;
    }

    if (left_pid == 0) {
        if (dup2(pipe_fds[1], STDOUT_FILENO) == -1) {
            fprintf(stderr, "dcsh: dup2: %s\n", strerror(errno));
            _exit(1);
        }

        close_pipe_fds(pipe_fds);
        run_child_command(left_argv, left_input_path, left_output_path);
    }

    right_pid = fork();

    if (right_pid == -1) {
        perror("dcsh: fork");
        close_pipe_fds(pipe_fds);
        wait_for_child(left_pid);
        return -1;
    }

    if (right_pid == 0) {
        if (dup2(pipe_fds[0], STDIN_FILENO) == -1) {
            fprintf(stderr, "dcsh: dup2: %s\n", strerror(errno));
            _exit(1);
        }

        close_pipe_fds(pipe_fds);
        run_child_command(right_argv, right_input_path, right_output_path);
    }

    close_pipe_fds(pipe_fds);

    left_status = wait_for_child(left_pid);
    right_status = wait_for_child(right_pid);

    if (left_status == -1 || right_status == -1) {
        return -1;
    }

    return right_status;
}

int execute_external_command(const TokenList *tokens)
{
    char *argv[DCSH_TOKEN_MAX + 1];
    const char *input_path;
    const char *output_path;
    pid_t child_pid;
    int pipe_index;

    if (tokens->count == 0) {
        return 0;
    }

    pipe_index = find_pipe_index(tokens);

    if (pipe_index == -2) {
        return -1;
    }

    if (pipe_index != -1) {
        return execute_pipeline(tokens, pipe_index);
    }

    if (parse_command(tokens, argv, &input_path, &output_path) != 0) {
        return -1;
    }

    child_pid = fork();

    if (child_pid == -1) {
        perror("dcsh: fork");
        return -1;
    }

    if (child_pid == 0) {
        run_child_command(argv, input_path, output_path);
    }

    return wait_for_child(child_pid);
}
