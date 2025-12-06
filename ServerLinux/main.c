/*
    File name: test.c
    Created at: 02-12-25
    Author: Solomon
*/

#include <stdio.h>
#include <string.h>
#include "Server.h"

//////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void launch(struct Server* server)
{
    char buffer[30000];
    printf("===== WAITING FOR CONNECTION =====\n");

    int address_length = sizeof(server->address);

    int new_socket = accept(server->socket,
                           (struct sockaddr*)&server->address,
                           (socklen_t*)&address_length);

    if (new_socket < 0)
    {
        printf("Accept failed.\n");
        return;
    }

    int bytes_received = recv(new_socket, buffer, 30000 - 1, 0);
    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0'; // ensure safe printing
        printf("%s\n", buffer);
    }

    char* hello =
        "HTTP/1.1 200 OK\r\n"
        "Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n"
        "Server: WinSock-Test-Server\r\n"
        "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
        "Content-Length: \r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>SucessFUl </h1></body></html>";

    send(new_socket, hello, strlen(hello), 0);

    close(new_socket);
}

////////////////////////////////////////////////////
///////////////////////////////////////////////////
int main()
{
    struct Server server =
        server_constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 8080, 10, launch);

    server.launch(&server);

    close(server.socket);

    printf("Hello World");

    return 0;
}

