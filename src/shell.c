#include <stdio.h>
#include <string.h>

#include "dcsh.h"

static void trim_trailing_newline(char *line)
{
    size_t length = strlen(line);

    if (length > 0 && line[length - 1] == '\n') {
        line[length - 1] = '\0';
    }
}

DcshStatus dcsh_run(void)
{
    char line[DCSH_LINE_MAX];

    while (1) {
        fputs(DCSH_PROMPT, stdout);
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            fputc('\n', stdout);
            return DCSH_OK;
        }

        trim_trailing_newline(line);

        if (line[0] == '\0') {
            continue;
        }

        if (strcmp(line, "exit") == 0) {
            return DCSH_OK;
        }

        printf("dcsh: command execution not implemented yet: %s\n", line);
    }
}