#include "server.h"

#define PORT 8080

int main(void)
{
    const int listenfd = server_listen_inet(PORT);
    return 0;
}