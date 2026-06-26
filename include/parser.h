#ifndef PARSER_H
#define PARSER_H

#include "dcsh.h"

typedef struct {
    char *items[DCSH_TOKEN_MAX];
    int count;
} TokenList;

int parse_tokens(char *line, TokenList *tokens);

#endif