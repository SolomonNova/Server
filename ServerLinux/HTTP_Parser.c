#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "HTTP_PARSER.h"

/////////////////////////////////////////
/////////////////////////////////////////
char** tokenizer(const char* s)
{
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
            {
                buffer[k++] = s[i++];
            }

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
    int subSize = j - i + 1;

    // find the colon ':' position k which seperates key and value pair for header

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

    char* buffer = (char*)malloc(endIndex * sizeof(char));

    for (int i = 0; i < endIndex; i++)
    {
        buffer[i] = request[i];
    }

    buffer[endIndex] = '\0';

    char** info = tokenizer(buffer);

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
            keysCollection[count] = getLineKeyValue(request, i, j - 1);
            j += 2;
            i = j;
            count++;
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

    while (p)
    {
        result += *p;
        p++;
    }

    return (unsigned short)((result % 1024) / 2);
}

/////////////////////////////////////////
/////////////////////////////////////////
char* parseBody(const char* s, Request* request)
{
    int startIndex = 0;
    int len = (int)strlen(s);

    // find the index of the first character of Body

    for (int x = 0; x < len; x++)
    {

        if (s[x] == '\r' && s[x + 1] == '\n' &&
            s[x + 2] == '\r' && s[x + 3] == '\n')
        {
            {
                startIndex = x + 4;
            }
        }

    }

    if (request->headers->values[hashFunction("Content-Length")] > 0)
    {
        int bodyLength = 0;
        char* lenString = request->headers->values[hashFunction("Content-Length")];

        for (int x = 0; x < '\r'; x++)
        {
            if (x > '9' || x < '0') continue;

            bodyLength = bodyLength * 10 + lenString[x - '0'];
        }

        char buffer[bodyLength + 1];

        for (int x = startIndex; x < len; x++)
        {
            buffer[startIndex - x] = s[x];
        }

        return buffer;
    }

    else
    {

    }
}

char* parseChunckedBody(char* s, int start, int end, int isLenAvailabe)
{
    if (isLenAvailabe == 0)
    {
        // find the start and end index of the hex size

    }
}




