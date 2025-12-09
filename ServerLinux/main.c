#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "Server.h"


int main()
{
    Server server = serverConstructor(AF_INET, SOCK_STREAM, IPPROTO_TCP, INADDR_ANY, 8080, 10, launch);
    server.launch(&server);
    close(server.socket_fd);

    printf("main is executed\n");

    return 0;
}