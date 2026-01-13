/*
    File name: HTTP_PARSER.c
    Creation date: 10-12-25
    Author: Solomon
*/

#define _POSIX_C_SOURCE 200809L // for strtok_r which is a POSIX function not standard function

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

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
PARSE_RESULT parse_request_line(REQUEST_INFO* ri_requestInfo, char* szRequestBuffer)
{
    if (!ri_requestInfo || !szRequestBuffer) return ERR_EMPTY_REQUEST;

    ri_requestInfo->m_pRequestStart = szRequestBuffer; // set the pointers in struct

    size_t iReqLength = strlen(szRequestBuffer);
    // 1. Ensure we only look at the first line
    char* pLineEnd = strstr(szRequestBuffer, "\r\n");
    if (!pLineEnd) return ERR_NULL_CHECK_FAILED;

    size_t iOffset = pLineEnd - szRequestBuffer + 2;
    if (iOffset > iReqLength) return ERR_INVALID_FORMAT;

    ri_requestInfo->m_pHeadersStart = pLineEnd + 2; // set the members of the struct

    // Terminate the first line so strtok doesn't bleed into headers
    *pLineEnd = '\0';

    char* pSavePtr;

    ri_requestInfo->m_szMethod = strtok_r(szRequestBuffer, " ", &pSavePtr);
    ri_requestInfo->m_szPath = strtok_r(NULL, " ", &pSavePtr);
    ri_requestInfo->m_szVersion = strtok_r(NULL, " ", &pSavePtr);

    if (!ri_requestInfo->m_szMethod) return ERR_INVALID_METHOD;
    if (!ri_requestInfo->m_szPath) return ERR_INVALID_PATH;
    if (!ri_requestInfo->m_szVersion) return ERR_INVALID_PROTOCOL;

    // no space after version
    if (strtok_r(NULL, " ", &pSavePtr) != NULL) {
        return ERR_INVALID_FORMAT; // Too many spaces/parameters
    }

    *pLineEnd = '\r';

    return PARSE_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
PARSE_RESULT parse_headers(REQUEST_INFO* ri_requestInfo, char* szRequestBuffer)
{
    if (!ri_requestInfo || !szRequestBuffer) return ERR_EMPTY_REQUEST;

    char* pRequestLineEnd = strstr(szRequestBuffer, "\r\n");
    if (!pRequestLineEnd) return ERR_INVALID_FORMAT;

    char* pHeadersStart = pRequestLineEnd + 2;
    char* pHeadersEnd = strstr(pHeadersStart, "\r\n\r\n");
    if (!pHeadersEnd) return ERR_INVALID_FORMAT;

    size_t iLength = strlen(pHeadersStart);
    size_t iOffset = pHeadersEnd - pHeadersStart + 4;

    if (iOffset > iLength) return ERR_INVALID_FORMAT; // safey for illegal acess
    ri_requestInfo->m_pBodyStart = pHeadersEnd + 4; // fill the struct

    *pHeadersEnd = '\0'; // Seal the header block


    char* pCurrentLine = pHeadersStart;
    HEADERS* h_entries = ri_requestInfo->m_h_headers;

    while (pCurrentLine && *pCurrentLine != '\0') {
        if (h_entries->iCount >= h_entries->iCapacity) return ERR_OUT_OF_BOUNDS;

        char* pColon = strchr(pCurrentLine, ':');
        if (!pColon) break; // Or handle as error: invalid header format

        *pColon = '\0';
        h_entries->hkv_arrEntries[h_entries->iCount].szKey = pCurrentLine;

        // Move to value and skip leading spaces
        char* pValue = pColon + 1;
        while (*pValue == ' ') pValue++;

        h_entries->hkv_arrEntries[h_entries->iCount].szValue = pValue;

        char* pLineEnd = strstr(pValue, "\r\n");
        if (pLineEnd)
        {
            *pLineEnd = '\0';
            pCurrentLine = pLineEnd + 2; // Move to next line
        }

        else
        {
            pCurrentLine = NULL; // End of string
        }

        h_entries->iCount++;
    }

    return PARSE_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
PARSE_RESULT parse_body(REQUEST_INFO* ri_requestInfo, char* szRequestBuffer)
{
    /*
        This function actually does not parse the body but formats
        the bytes of the body / extract the bytes from the body with
        different transfer encoding.
        The actual parsing depends on "Content-Type" header of the body
        and will be done later
    */

    if (!ri_requestInfo || !szRequestBuffer) return ERR_NULL_CHECK_FAILED;
    if (!ri_requestInfo->m_pBodyStart) return ERR_INVALID_FORMAT;

    char* szContentLength = NULL;
    char* szTransferEncoding = NULL;

    for (size_t iX = 0; iX < ri_requestInfo->m_h_headers->iCount; iX++)
    {
        if (strcmp(ri_requestInfo->m_h_headers->hkv_arrEntries[iX].szKey, "Content-Length") == 0)
            szContentLength = ri_requestInfo->m_h_headers->hkv_arrEntries[iX].szValue;

        if (strcmp(ri_requestInfo->m_h_headers->hkv_arrEntries[iX].szKey, "Transfer-Encoding") == 0)
            szContentLength = ri_requestInfo->m_h_headers->hkv_arrEntries[iX].szValue;
    }


    // set the field of structs
    if (!szContentLength) return ERR_NULL_CHECK_FAILED;
    size_t iBodyLength = (size_t)(szContentLength);
    ri_requestInfo->m_iBodyLength = iBodyLength;

    // when encoding is chuncked
    if (strcmp(szTransferEncoding, "chunked") == 0);



    return PARSE_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
PARSE_RESULT decode_chunked_body(REQUEST_INFO* ri_requestInfo, char* pBodyStart)
{
    if (!ri_requestInfo || !pBodyStart) return ERR_NULL_CHECK_FAILED;

    size_t iCount = 0;
    size_t iCapacity = 1024;
    char* pBodyBuffer = calloc(iCapacity, 1);
    char* pEnd = ri_requestInfo->m_pOriginalRequest + ri_requestInfo->m_iTotalRawBytes;

    char* pCurrentByte = pBodyStart;
    size_t iChunkSize = 0;
    do
    {
        iChunkSize = 0;
        int iHexChars = 0;

        while ((pCurrentByte + 1 < pEnd) && *pCurrentByte != '\r')
        {
            if (++iHexChars > 16) goto safe_return; // Max 16 hex chars for 64-bit size_t

            if (*pCurrentByte >= '0' && *pCurrentByte <= '9')
                iChunkSize = 16 * iChunkSize + (*pCurrentByte - '0');
            else if (*pCurrentByte >= 'A' && *pCurrentByte <= 'F')
                iChunkSize = 16 * iChunkSize + (10 + (*pCurrentByte - 'A'));
            else if (*pCurrentByte >= 'a' && *pCurrentByte <= 'f')
                iChunkSize = 16 * iChunkSize + (10 + (*pCurrentByte - 'a'));
            else goto safe_return;

            pCurrentByte++;
        }

        // MODIFIED: Ensure we aren't at the very end of the buffer before checking CRLF
        if (pCurrentByte + 1 >= pEnd) goto safe_return;

        if (iChunkSize == 0) break;

        if (iCount + iChunkSize > 0x00A00000ULL) goto safe_return;
        while (!(iCount + iChunkSize < iCapacity))
        {
            iCapacity *= 2;
            char* pTempBuffer = realloc(pBodyBuffer, iCapacity);
            if (!pTempBuffer) { free(pBodyBuffer); return ERR_NULL_CHECK_FAILED; };
            pBodyBuffer = pTempBuffer;
        }

        // Verify CRLF after the hex size
        if (!(*pCurrentByte == '\r' && *(pCurrentByte + 1) == '\n')) goto safe_return;
        pCurrentByte += 2;

        // We check for iChunkSize + 2 to account for the trailing \r\n
        if (pCurrentByte + iChunkSize + 2 > pEnd) goto safe_return;

        for (size_t iX = 0; iX < iChunkSize; iX++)
            pBodyBuffer[iCount++] = *(pCurrentByte++);

        // Verify the mandatory CRLF that must follow every data chunk
        if (!(*pCurrentByte == '\r' && *(pCurrentByte + 1) == '\n')) goto safe_return;
        pCurrentByte += 2;

    } while (iChunkSize != 0);

    // Correctly handle the final CRLF of the 0 chunk (0\r\n\r\n)
    if (pCurrentByte + 1 >= pEnd)
        goto safe_return;

    // No trailers: immediate empty line
    if (*pCurrentByte == '\r' && *(pCurrentByte + 1) == '\n')
    {
        pCurrentByte += 2;
        ri_requestInfo->m_pRequestEnd = pCurrentByte;
    }

    else // else executes if there are trailers behind body section
    {
        while (1)
        {
            // Need at least 2 bytes to check for CRLF
            if (pCurrentByte + 1 >= pEnd) goto safe_return;

            // Empty line marks end of trailers
            if (*pCurrentByte == '\r' && *(pCurrentByte + 1) == '\n')
            {
                pCurrentByte += 2;
                break;
            }

            // Skip current trailer line until CRLF
            while (pCurrentByte + 1 < pEnd &&
                !(*pCurrentByte == '\r' && *(pCurrentByte + 1) == '\n'))
            {
                pCurrentByte++;
            }

            if (pCurrentByte + 1 >= pEnd) goto safe_return;

            // Consume CRLF of this trailer line
            pCurrentByte += 2;
        }

        ri_requestInfo->m_pRequestEnd = pCurrentByte;
    }


    char* pTemp = realloc(pBodyBuffer, iCount + 1);
    if (!pTemp) goto safe_return;
    pTemp[iCount] = '\0';
    pBodyBuffer = pTemp;

    ri_requestInfo->m_iBodyLength = iCount;
    ri_requestInfo->m_szBody = pBodyBuffer;

    return PARSE_SUCCESS;

    safe_return:
        if (pBodyBuffer) free(pBodyBuffer);
        return ERR_INVALID_FORMAT;
}

////////////////////////////////////////////////////////////
//==================== HELPER FUNCTIONS ===================//
////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
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
