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

int main() {
    // Write C code here
    char szSource[] = "/this/is/a/test";
    char** arrItems = tokenizer(szSource, '/');

    for (size_t iX = 0; arrItems[iX] != NULL; iX++)
    {
        printf("%s\n", arrItems[iX]);
    }

    return 0;
}

REQUEST_INFO* fillRequest(const char* s)
{
    REQUEST_INFO* request = (REQUEST_INFO*)calloc(1, sizeof(REQUEST_INFO));

    char** methodBuffer = parseMethod(s);
    if (!methodBuffer) return NULL;

    memcpy(request->method, methodBuffer[0], strlen(methodBuffer[0]));
    memcpy(request->path, methodBuffer[1], strlen(methodBuffer[1]));
    memcpy(request->version, methodBuffer[2], strlen(methodBuffer[2]));

    request->headers = parseHeaders(s);

    request->body = parseBody(s, request);

    return request;
}

/////////////////////////////////////////
/////////////////////////////////////////
void printRequest(REQUEST_INFO* request)
{
    printf("Method: %s\n", request->method);
    printf("Path: %s\n", request->path);
    printf("HTTP version: %s\n", request->version);

    for (int x = 0; request->headers->keys[x] != NULL; x++)
    {
        printf("%s: ", request->headers->keys[x]);
        printf("%s\n", request->headers->values[hashFunction(request->headers->keys[x])]);
    }

    printf("\n\n%s", request->body);
}

/////////////////////////////////////////
/////////////////////////////////////////
PARSE_RESULT parse_request_line(REQUEST_INFO* ri_requestInfo, char* szRequest)
{
    /*
        reads the  http request and parses the request
        line (ie: method, path, and http version)
    */

    if (!ri_requestInfo || !szRequest) return NULL;

    // extract the first line
    char* pLineEnd = strstr(szRequest, "\r\n");
    if (!pLineEnd) return NULL;

    size_t iLineLength = pLineEnd - szRequest;
    char* szFirstLine = calloc(iLineLength + 1, 1);
    if (!szFirstLine) return NULL;
    memcpy(szFirstLine, szRequest, iLineLength);

    // tokenize
    char** arrTokens = tokenizer(szFirstLine, ' ');
    if (!arrTokens)
    {
        free(szFirstLine);
        return NULL;
    }



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
char** tokenizer(const char* szSource, const char cDelimiter)
{
    /*
        returns a array containing pointers to strings
        that were seperated by delimiter in the source string.

        NOTE: strtok() modifies the source string so a duplicate string
              must be fed to it
    */

    if (szSource == NULL) return NULL;

    size_t iSourceLength = strlen(szSource);
    char* szSourceDup = calloc(iSourceLength + 1, 1); // duplicate modifiable string, don't use VLA
    if (!szSourceDup) return NULL;
    strcpy(szSourceDup, szSource);

    const char szDelimiter[2] = { cDelimiter, '\0' };
    size_t iCapacity = 8; // capacity of returned array
    size_t iCount = 0;

    char** arrTokens = calloc(iCapacity, sizeof(char*));
    if (!arrTokens) return NULL;

    char* pToken = strtok(szSourceDup, szDelimiter);
    while (pToken)
    {
        if (iCount + 1 >= iCapacity)
        {
            iCapacity *= 2;

            char** pTemp = realloc(arrTokens, iCapacity * sizeof(char*));
            if (!pTemp) goto cleanup;

            arrTokens = pTemp;
        }

        size_t iBufferLength = strlen(pToken);
        char* pBuffer = calloc(iBufferLength + 1, 1);
        if (!pBuffer) goto cleanup;

        memcpy(pBuffer, pToken, iBufferLength);
        arrTokens[iCount++] = pBuffer;

        pToken = strtok(NULL, szDelimiter);
    }

    arrTokens[iCount] = NULL;
    char** pTemp = realloc(arrTokens, (iCount + 1) * sizeof(char*)); // realloc to the final size
    free(szSourceDup);

    return pTemp ? pTemp : arrTokens;

    // cleanup label when memroy allocation fails
cleanup:
    for (int iX = 0; iX < iCount; iX++)
        free(arrTokens[iX]);
    free(arrTokens);
    free(szSourceDup);
    return NULL;
}

/////////////////////////////////////////
/////////////////////////////////////////
void tokenizer_cleanup(char** arrTokens)
{
    if (arrTokens == NULL) return;

    for (size_t iX = 0; arrTokens[iX] != NULL; iX++)
    {
        free(arrTokens[iX]);
    }

    free(arrTokens);
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
