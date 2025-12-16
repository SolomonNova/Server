/*
    File name: HTTP_PARSER.h
    Creation date: 10-12-25
    Author: Solomon
*/

#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

typedef struct Headers
{
    char* keys[64];
    char* values[1024];
} Headers;

typedef struct Request
{
    char method[8];
    char path[1024];
    char version[8];
    Headers* headers;
    char* body;
} Request;

char** parseMethod(const char* request);
Headers* parseHeaders(const char* request);
char* parseBody(const char* s, Request* request);

//============Helper Functions================//

char** tokenizer(const char* s);
/*
    Takes a string and returns an array of pointers
    to its substrings that were seperated by a space character ' '.
*/

char** getLineKeyValue(const char* s, int i, int j);
/*
    Takes a string with the last 2 characters as '\r' and '\n' and
    a single colon ':' anywhere between i and j index.
    Note: i and j are 0th and second last characters when considiring
    the null terminiator as a part of the string.
*/

unsigned short hashFunction(const char* s);

char* getValue(const char* s, Headers* headers);
/*
    Gets the value corrosponding the key string s
*/

char* parseChunckedBody(const char* s, int startIndex);

#endif