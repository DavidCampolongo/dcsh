#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dcsh.h"
#include "executor.h"
#include "parser.h"

static void trim_trailing_newline(char *line)
{
    size_t length = strlen(line);

    if (length > 0 && line[length - 1] == '\n') {
        line[length - 1] = '\0';
    }
}

static void print_help(void)
{
    puts("dcsh built-ins:");
    puts("  cd [directory]  change the shell working directory");
    puts("  pwd             print the shell working directory");
    puts("  help            show this help message");
    puts("  exit            exit dcsh");
}

static int execute_cd(const TokenList *tokens)
{
    const char *path;

    if (tokens->count > 2) {
        fprintf(stderr, "dcsh: cd: too many arguments\n");
        return 1;
    }

    if (tokens->count == 1) {
        path = getenv("HOME");

        if (path == NULL) {
            fprintf(stderr, "dcsh: cd: HOME not set\n");
            return 1;
        }
    } else {
        path = tokens->items[1];
    }

    if (chdir(path) != 0) {
        fprintf(stderr, "dcsh: cd: %s: %s\n", path, strerror(errno));
        return 1;
    }

    return 1;
}

static int execute_pwd(const TokenList *tokens)
{
    char cwd[DCSH_LINE_MAX];

    if (tokens->count != 1) {
        fprintf(stderr, "dcsh: pwd: too many arguments\n");
        return 1;
    }

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "dcsh: pwd: %s\n", strerror(errno));
        return 1;
    }

    puts(cwd);
    return 1;
}

static int execute_help(const TokenList *tokens)
{
    if (tokens->count != 1) {
        fprintf(stderr, "dcsh: help: too many arguments\n");
        return 1;
    }

    print_help();
    return 1;
}

static int execute_builtin(const TokenList *tokens)
{
    if (strcmp(tokens->items[0], "cd") == 0) {
        return execute_cd(tokens);
    }

    if (strcmp(tokens->items[0], "pwd") == 0) {
        return execute_pwd(tokens);
    }

    if (strcmp(tokens->items[0], "help") == 0) {
        return execute_help(tokens);
    }

    return 0;
}

DcshStatus dcsh_run(void)
{
    char line[DCSH_LINE_MAX];

    while (1) {
        TokenList tokens;

        fputs(DCSH_PROMPT, stdout);
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            fputc('\n', stdout);
            return DCSH_OK;
        }

        trim_trailing_newline(line);

        if (parse_tokens(line, &tokens) != 0) {
            fprintf(stderr, "dcsh: too many tokens\n");
            continue;
        }

        if (tokens.count == 0) {
            continue;
        }

        if (tokens.count == 1 && strcmp(tokens.items[0], "exit") == 0) {
            return DCSH_OK;
        }

        if (execute_builtin(&tokens)) {
            continue;
        }

        execute_external_command(&tokens);
    }
}
