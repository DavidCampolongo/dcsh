#include <ctype.h>
#include <stddef.h>

#include "parser.h"

static int is_operator(char ch)
{
    return ch == '|' || ch == '<' || ch == '>';
}

static char *operator_token(char ch)
{
    if (ch == '|') {
        return "|";
    }

    if (ch == '<') {
        return "<";
    }

    if (ch == '>') {
        return ">";
    }

    return NULL;
}

static int add_token(TokenList *tokens, char *token)
{
    if (tokens->count >= DCSH_TOKEN_MAX) {
        return -1;
    }

    tokens->items[tokens->count] = token;
    tokens->count++;

    return 0;
}

int parse_tokens(char *line, TokenList *tokens)
{
    char *cursor = line;

    tokens->count = 0;

    while (*cursor != '\0') {
        while (isspace((unsigned char)*cursor)) {
            *cursor = '\0';
            cursor++;
        }

        if (*cursor == '\0') {
            break;
        }

        if (is_operator(*cursor)) {
            if (add_token(tokens, operator_token(*cursor)) != 0) {
                return -1;
            }

            cursor++;
            continue;
        }

        char *start = cursor;

        while (*cursor != '\0' &&
               !isspace((unsigned char)*cursor) &&
               !is_operator(*cursor)) {
            cursor++;
        }

        if (*cursor == '\0') {
            if (add_token(tokens, start) != 0) {
                return -1;
            }

            break;
        }

        char separator = *cursor;
        *cursor = '\0';

        if (add_token(tokens, start) != 0) {
            return -1;
        }

        if (is_operator(separator)) {
            if (add_token(tokens, operator_token(separator)) != 0) {
                return -1;
            }
        }

        cursor++;
    }

    return 0;
}