/*
    File name: HTTP_PARSER.c
    Creation date: 10-12-25
    Author: Solomon
*/

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "HTTP_PARSER.h"

////////////////////////////////////////////////////////////
//==================== MAIN PARSER API ====================//
////////////////////////////////////////////////////////////

/////////////////////////////////////////
/////////////////////////////////////////
char** parseMethod(const char* request)
{
    unsigned int len = strlen(request);
    unsigned int endIndex = 0;

    for (int i = 0; i < len; i++)
    {
        if (request[i] == '\r')
        {
            endIndex = i;
            break;
        }
    }

    char* buffer = (char*)malloc(endIndex + 1);

    for (int i = 0; i < endIndex; i++)
        buffer[i] = request[i];

    buffer[endIndex] = '\0';

    char** info = tokenizer(buffer);
    free(buffer);

    return info;
}

/////////////////////////////////////////
/////////////////////////////////////////
Headers* parseHeaders(const char* request)
{
    unsigned int len = strlen(request);
    int startIndex = -1;
    int endIndex = -1;

    for (int x = 0; x + 1 < len; x++)
        if (request[x] == '\r' && request[x + 1] == '\n')
        {
            startIndex = x + 2;
            break;
        }

    if (startIndex == -1) return NULL;

    for (int x = startIndex; x + 3 < len; x++)
        if (request[x] == '\r' && request[x + 1] == '\n' &&
            request[x + 2] == '\r' && request[x + 3] == '\n')
        {
            endIndex = x - 1;
            break;
        }

    if (endIndex == -1) return NULL;

    int i = startIndex;
    int j = startIndex;
    int count = 0;

    char*** keysCollection = (char***)calloc(64, sizeof(char**));

    while (j <= endIndex)
    {
        if (request[j] == '\r')
        {
            keysCollection[count++] = getLineKeyValue(request, i, j - 1);
            j += 2;
            i = j;
            continue;
        }
        j++;
    }

    Headers* headers = (Headers*)calloc(1, sizeof(Headers));

    for (int x = 0; x < count; x++)
    {
        char* key = keysCollection[x][0];
        char* value = keysCollection[x][1];

        headers->keys[x] = key;

        unsigned short index = hashFunction(key);
        while (headers->values[index] != NULL)
            index = (index + 1) % 1024;

        headers->values[index] = value;
    }

    headers->keys[count] = NULL;

    free(keysCollection);
    return headers;
}

/////////////////////////////////////////
/////////////////////////////////////////
char* parseBody(const char* s, Request* request)
{
    int startIndex = 0;
    int len = (int)strlen(s);

    for (int x = 0; x + 3 < len; x++)
    {
        if (s[x] == '\r' && s[x + 1] == '\n' &&
            s[x + 2] == '\r' && s[x + 3] == '\n')
        {
            startIndex = x + 4;
            break;
        }
    }

    char* lenString = getValue("Content-Length", request->headers);

    if (lenString != NULL)
    {
        int bodyLength = 0;

        for (int x = 0; lenString[x] != '\0'; x++)
        {
            if (lenString[x] < '0' || lenString[x] > '9') break;
            bodyLength = bodyLength * 10 + (lenString[x] - '0');
        }

        char* buffer = (char*)calloc(bodyLength + 1, sizeof(char));

        for (int i = 0; i < bodyLength; i++)
            buffer[i] = s[startIndex + i];

        buffer[bodyLength] = '\0';
        return buffer;
    }

    return parseChunckedBody(s, startIndex);
}

////////////////////////////////////////////////////////////
//==================== HELPER FUNCTIONS ===================//
////////////////////////////////////////////////////////////

/////////////////////////////////////////
/////////////////////////////////////////
char** tokenizer(const char* s)
{
    // definition in header

    int len = strlen(s);
    int tokenCount = 1;

    for (int x = 0; x < len; x++)
        if (s[x] == ' ') tokenCount++;

    char** res = malloc((tokenCount + 1) * sizeof(char*));

    int i = 0;
    int j = 0;
    int count = 0;

    while (j <= len)
    {
        if (s[j] == ' ' || s[j] == '\0')
        {
            int size = j - i;
            char* buffer = malloc(size + 1);

            int k = 0;
            while (i < j)
                buffer[k++] = s[i++];

            buffer[k] = '\0';
            res[count++] = buffer;
            i = j + 1;
        }
        j++;
    }

    res[count] = NULL;
    return res;
}

/////////////////////////////////////////
/////////////////////////////////////////
char** getLineKeyValue(const char* s, int i, int j)
{
    // definition in header

    int subSize = j - i + 1;
    int k = 0;

    for (int x = 0; x < subSize; x++)
        if (s[i + x] == ':') k = i + x;

    char* key = (char*)calloc(k - i + 1, sizeof(char));
    char* value = (char*)calloc(j - k + 1, sizeof(char));

    for (int x = 0; x < k - i; x++)
        key[x] = s[i + x];

    for (int x = 0; x < j - k; x++)
        value[x] = s[k + x + 1];

    char** result = (char**)calloc(3, sizeof(char*));
    result[0] = key;
    result[1] = value;

    return result;
}

/////////////////////////////////////////
/////////////////////////////////////////
char* getValue(const char* s, Headers* headers)
{
    unsigned short index = hashFunction(s);

    while (headers->values[index] != NULL)
    {
        if (strcmp(headers->keys[index], s) == 0)
            return headers->values[index];

        index = (index + 1) % 1024;
    }

    return NULL;
}

/////////////////////////////////////////
/////////////////////////////////////////
unsigned short hashFunction(const char* s)
{
    unsigned int result = 0;
    const char* p = s;

    while (*p)
        result += *p++;

    return (unsigned short)((result % 1024) / 2);
}

/////////////////////////////////////////
/////////////////////////////////////////
char* parseChunckedBody(const char* s, int startIndex)
{
    int len = (int)strlen(s);
    char* body = (char*)calloc(4096, sizeof(char));
    int bodyIndex = 0;
    int x = startIndex;

    while (x < len)
    {
        int size = 0;

        while (s[x] != '\r')
        {
            if (s[x] >= '0' && s[x] <= '9')
                size = size * 16 + (s[x] - '0');
            else if (s[x] >= 'A' && s[x] <= 'F')
                size = size * 16 + (10 + s[x] - 'A');
            else if (s[x] >= 'a' && s[x] <= 'f')
                size = size * 16 + (10 + s[x] - 'a');
            x++;
        }

        if (size == 0) break;

        x += 2;

        for (int i = 0; i < size; i++)
            body[bodyIndex++] = s[x++];

        x += 2;
    }

    body[bodyIndex] = '\0';
    return body;
}
