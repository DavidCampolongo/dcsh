#include <stdio.h>
#include <string.h>

#include "dcsh.h"
#include "parser.h"

static void trim_trailing_newline(char *line)
{
    size_t length = strlen(line);

    if (length > 0 && line[length - 1] == '\n') {
        line[length - 1] = '\0';
    }
}

static void print_tokens(const TokenList *tokens)
{
    printf("tokens (%d):\n", tokens->count);

    for (int i = 0; i < tokens->count; i++) {
        printf("  [%d] %s\n", i, tokens->items[i]);
    }
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

        print_tokens(&tokens);
    }
}