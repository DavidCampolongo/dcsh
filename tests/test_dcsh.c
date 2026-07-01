#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dcsh.h"
#include "executor.h"
#include "parser.h"

static int failures = 0;

static void check(int condition, const char *name)
{
    if (condition) {
        printf("PASS: %s\n", name);
        return;
    }

    fprintf(stderr, "FAIL: %s\n", name);
    failures++;
}

static int parse_line(const char *line, char buffer[], TokenList *tokens)
{
    snprintf(buffer, DCSH_LINE_MAX, "%s", line);
    return parse_tokens(buffer, tokens);
}

static int run_line(const char *line)
{
    char buffer[DCSH_LINE_MAX];
    TokenList tokens;

    if (parse_line(line, buffer, &tokens) != 0) {
        return -1;
    }

    return execute_external_command(&tokens);
}

static int make_temp_path(char path[], size_t path_size)
{
    int fd;

    snprintf(path, path_size, "/tmp/dcsh_test_XXXXXX");
    fd = mkstemp(path);

    if (fd == -1) {
        return -1;
    }

    close(fd);
    return 0;
}

static int write_text_file(const char *path, const char *text)
{
    FILE *file = fopen(path, "w");

    if (file == NULL) {
        return -1;
    }

    if (fputs(text, file) == EOF) {
        fclose(file);
        return -1;
    }

    return fclose(file);
}

static int read_text_file(const char *path, char buffer[], size_t buffer_size)
{
    FILE *file = fopen(path, "r");
    size_t bytes_read;

    if (file == NULL) {
        return -1;
    }

    bytes_read = fread(buffer, 1, buffer_size - 1, file);
    buffer[bytes_read] = '\0';

    if (ferror(file)) {
        fclose(file);
        return -1;
    }

    return fclose(file);
}

static void test_parser_splits_operators(void)
{
    char buffer[DCSH_LINE_MAX];
    TokenList tokens;

    check(parse_line("cat<input|wc>output", buffer, &tokens) == 0,
          "parse compact operators");
    check(tokens.count == 7, "operator token count");
    check(strcmp(tokens.items[0], "cat") == 0, "first command token");
    check(strcmp(tokens.items[1], "<") == 0, "input operator token");
    check(strcmp(tokens.items[2], "input") == 0, "input file token");
    check(strcmp(tokens.items[3], "|") == 0, "pipe operator token");
    check(strcmp(tokens.items[4], "wc") == 0, "second command token");
    check(strcmp(tokens.items[5], ">") == 0, "output operator token");
    check(strcmp(tokens.items[6], "output") == 0, "output file token");
}

static void test_output_redirection(void)
{
    char output_path[64];
    char command[DCSH_LINE_MAX];
    char contents[128];

    check(make_temp_path(output_path, sizeof(output_path)) == 0,
          "create output temp path");
    snprintf(command, sizeof(command), "echo hello > %s", output_path);

    check(run_line(command) == 0, "execute output redirection");
    check(read_text_file(output_path, contents, sizeof(contents)) == 0,
          "read redirected output");
    check(strcmp(contents, "hello\n") == 0, "output redirection contents");

    unlink(output_path);
}

static void test_input_and_output_redirection(void)
{
    char input_path[64];
    char output_path[64];
    char command[DCSH_LINE_MAX];
    char contents[128];

    check(make_temp_path(input_path, sizeof(input_path)) == 0,
          "create input temp path");
    check(make_temp_path(output_path, sizeof(output_path)) == 0,
          "create copy temp path");
    check(write_text_file(input_path, "from-file\n") == 0,
          "write input temp file");

    snprintf(command, sizeof(command), "cat < %s > %s", input_path, output_path);

    check(run_line(command) == 0, "execute input and output redirection");
    check(read_text_file(output_path, contents, sizeof(contents)) == 0,
          "read copied output");
    check(strcmp(contents, "from-file\n") == 0,
          "input redirection copied contents");

    unlink(input_path);
    unlink(output_path);
}

static void test_single_pipe(void)
{
    char output_path[64];
    char command[DCSH_LINE_MAX];
    char contents[128];

    check(make_temp_path(output_path, sizeof(output_path)) == 0,
          "create pipe temp path");
    snprintf(command, sizeof(command), "printf abc | wc -c > %s", output_path);

    check(run_line(command) == 0, "execute single pipe");
    check(read_text_file(output_path, contents, sizeof(contents)) == 0,
          "read pipe output");
    check(strchr(contents, '3') != NULL, "pipe output byte count");

    unlink(output_path);
}

int main(void)
{
    test_parser_splits_operators();
    test_output_redirection();
    test_input_and_output_redirection();
    test_single_pipe();

    if (failures != 0) {
        fprintf(stderr, "%d test(s) failed\n", failures);
        return 1;
    }

    puts("All tests passed");
    return 0;
}
