#ifndef DCSH_H
#define DCSH_H

#define DCSH_LINE_MAX 1024
#define DCSH_TOKEN_MAX 64
#define DCSH_PROMPT "dcsh> "

typedef enum {
    DCSH_OK = 0,
    DCSH_EXIT = 1,
    DCSH_ERROR = 2
} DcshStatus;

DcshStatus dcsh_run(void);

#endif