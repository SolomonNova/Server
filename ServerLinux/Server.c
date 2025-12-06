/*
    File name: Server.c
    Createad at: 06-12-2025
    Author: Solomon
*/

#include<netinet/in.h>
#include<string.h>
#include<sys/socket.h>
#include<stdlib.h> // exit()
#include<stdio.h> // perror()
#include"Server.h"

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