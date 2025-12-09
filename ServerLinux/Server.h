#ifndef Server_H
#define Server_H

#include <sys/socket.h> // domain macros, service macros
#include <netinet/in.h> // protocol macros, interface address macros

/*
* @brief Represents a server configuration
*
* @param - domain      Address faimly (AF_INET, AF_INET6)    -> tells which ip version to use in the server.

* @param - service     Socket type (SOCK_STREAM, SOCK_DGRAM) -> SOCK_STREAM type socket is always used with TCP, SOCK_DGRAM is always used with UDP.

* @param - protocol    Protocol (IPPROTO_TCP, IPPROTO_UDP)   -> protocol to be used, either TCP or UDP.

* @param - interface   Inteface Address (INADDR_ANY          -> tells which network inteface in this system must the server be bound to.

* @param - port        Port number                           -> Identifies unique service bounded to this number, there can be many ports in a system
                                                                each running a different.

* @param - address     struct sockaddr_in type address       -> Tells the OS, which address family (IPv4) server belongs to,
                                                                port number being which the server will listen to,
                                                                IP address of the network inteface that the port is attached on.

* @param - backlog     backlog number                        -> is the number of pending connections the kernel will queue after listen()
                                                                but before your server calls accept().
                                                                It means pending connections and does not limit the number of clients.
*/

typedef struct Server
{
    int domain;
    int service;
    int protocol;
    unsigned long interFace;
    int port;
    int backlog;
    int socket_fd;
    struct sockaddr_in address;
    void (*launch)(struct Server*);
} Server;

Server serverConstructor(int domain,
    int service,
    int protocol,
    unsigned long interface,
    int port,
    int backlog,
    void (*launch)(struct Server*));

void launch(Server* server);

#endif
