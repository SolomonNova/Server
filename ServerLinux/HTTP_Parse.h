#ifndef HTTP_Parse_H
#define HTTP_Parse_H

typedef struct Headers
{
    char keys[64];
    char* values[64];
} Headers;

typedef struct Request
{
    char method[8];
    char path[1024];
    char version[8];
    Headers* headers;
    char* body;
} Request;

char** parseMethod(char* request);
Headers* parseHeaders(char* request);
char** parseBody(char* request);


#endif