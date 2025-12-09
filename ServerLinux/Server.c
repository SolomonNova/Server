/*
    File name: Server.c
    Createad at: 06-12-2025
    Author: Solomon
*/

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h> // exit()
#include <stdio.h> // perror()
#include <unistd.h> // close(fd), fork()
#include "Server.h"

Server serverConstructor(int domain,
    int service,
    int protocol,
    unsigned long interface,
    int port,
    int backlog,
    void (*launch)(struct Server* server))
{
    Server server;

    server.domain = domain;
    server.service = service;
    server.protocol = protocol;
    server.interFace = interface;
    server.port = port;
    server.backlog = backlog;

    memset(&server.address, 0, sizeof(server.address));
    server.address.sin_family = domain;
    server.address.sin_port = htons(port);
    server.address.sin_addr.s_addr = htonl(interface);

    server.socket_fd = socket(domain, service, protocol);

    int opt = 1;
    setsockopt(server.socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (server.socket_fd < 0)
    {
        perror("Socket failed\n");
        exit(1);
    }

    if (bind(server.socket_fd, (struct sockaddr*)&server.address, sizeof(server.address)) < 0)
    {
        perror("Bind failed\n");
        exit(1);
    }

    if (listen(server.socket_fd, 10) < 0)
    {
        perror("Listen failed\n");
        exit(1);
    }

    server.launch = launch;

    return server;
}

void launch(Server* server)
{
    char buffer[30000];
    printf("====WAITING FOR CONNECTION====\n");
    int i = 0;

    while (1)
    {
        int addressLen = sizeof(server->address);

        int newSocket_fd = accept(server->socket_fd, (struct sockaddr*)&server->address, (socklen_t*)&addressLen);

        if (newSocket_fd < 0)
        {
            printf("New socket failed\n");
            return;
        }

        pid_t f = fork();

        if (f == 0)
        {
            int bytesRecieved = recv(newSocket_fd, buffer, 30000 - 1, 0);
            if (bytesRecieved > 0)
            {
                buffer[bytesRecieved] = '\0';
                printf("%s\n", buffer);
                printf("%d process\n", i + 1);
                i++;
            }

            char* response = "HTTP/1.1 200 OK\r\n"
                "Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n"
                "Server: Linux\r\n"
                "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
                "Content-Length: \r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<html><body><h1>SucessFUl from VS Codde </h1></body></html>";

            send(newSocket_fd, response, strlen(response), 0);

            close(newSocket_fd);
        }
    }

}
