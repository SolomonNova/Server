#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "HTTP_Parser.h"

/////////////////////////////////////////
/////////////////////////////////////////
char** tokenizer(char* s)
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
char** getLineKeyValue(char* s, int i, i)
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
char** parseMethod(char* request)
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
Headers* parseHeaders(char* request)
{
    // get the starting and ending index of the headers

    unsigned int len = strlen(request);

    int startIndex = 0;
    int endIndex = 0;

    for (int x = 0; x < len; x++)
    {
        if (request[x] == '\n') startIndex = x + 1;

        if (request[x] == '\n' && request[x + 1] == '\r' && request[x + 2] == '\n') endIndex = x - 1;
    }

    int i = 0;
    int j = 0;

    // to be finished
}