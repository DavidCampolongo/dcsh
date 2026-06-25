#include "dcsh.h"

int main(void)
{
    DcshStatus status = dcsh_run();

    if (status == DCSH_ERROR) {
        return 1;
    }

    return 0;
}