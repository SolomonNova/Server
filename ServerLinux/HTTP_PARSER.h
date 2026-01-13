/*
    File name: HTTP_PARSER.h
    Creation date: 10-12-25
    Author: Solomon
*/

#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

typedef struct HEADER_KEY_VALUE
{
    char* szKey;
    char* szValue;
} HEADER_KEY_VALUE;

typedef struct HEADERS // array of key-value pairs
{
    HEADER_KEY_VALUE* hkv_arrEntries;
    size_t            iCount;
    size_t            iCapacity;
} HEADERS;

typedef struct REQUEST_INFO
{
    // original request info
    const char* m_pOriginalRequest;  // pointer to the orignal request string
    size_t         m_iTotalRawBytes;    // tells how many raw bytes were received in the original http request

    // request-line info
    char* m_szMethod;  // pointer to szRequestBuffer (do not free)
    char* m_szPath;    // pointer to szRequestBuffer (do not free)
    char* m_szVersion; // pointer to szRequestBuffer (do not free)

    // headers info
    HEADERS* m_h_headers;

    // body info
    char* m_szBody;    // string containing the bytes of the body
    size_t         m_iBodyLength;
    HEADERS* m_h_trailerHeaders;

    // general info about the request
    char* m_pRequestStart;
    char* m_pHeadersStart;
    char* m_pBodyStart;
    char* m_pRequestEnd;
} REQUEST_INFO;

typedef enum
{
    PARSE_SUCCESS = 0,
    ERR_NULL_CHECK_FAILED,
    ERR_EMPTY_REQUEST,
    ERR_INVALID_METHOD,
    ERR_INVALID_PATH,
    ERR_INVALID_PROTOCOL,
    ERR_CALLOC_FAILED,
    ERR_INVALID_FORMAT,
    ERR_OUT_OF_BOUNDS,
} PARSE_RESULT;

PARSE_RESULT parse_request_line(REQUEST_INFO* ri_requestInfo, char* szRequestBuffer);
PARSE_RESULT parse_headers(REQUEST_INFO* ri_requestInfo, char* szRequestBuffer);
PARSE_RESULT parse_body(REQUEST_INFO* ri_requestInfo, char* szRequestBuffer);
PARSE_RESULT decode_chunked_body(REQUEST_INFO* ri_requestInfo, char* pBodyStart); // fills the REQUES_INFO struct if body is chunked (Transfer-Encoding: Chunked )
PARSE_RESULT parse_trailers(REQUEST_INFO* ri_requestInfo, char* pTrailerStart);

REQUEST_INFO* fillREQUEST_INFO(const char* s);

//============Helper Functions================//

void printREQUEST_INFO(REQUEST_INFO* request);

char** tokenizer(const char* s, const char cDelimiter);
/*
    Takes a string and returns an array of pointers
    to its substrings that were seperated by a space character ' '.
*/

void tokenizer_cleanup(char** arrItems);

char** getLineKeyValue(const char* s, int i, int j);
/*
    Takes a string with the last 2 characters as '\r' and '\n' and
    a single colon ':' anywhere between i and j index.
    Note: i and j are 0th and second last characters when considiring
    the null terminiator as a part of the string.
*/

unsigned short hashFunction(const char* s);
/*
    Gives integer to be used as a index for a header string
*/

char* getValue(const char* s, HEADERS* headers);
/*
    Gets the value corrosponding the key string s
*/

char* parseChunckedBody(const char* s, int startIndex);
/*
    find the chunk size and make a buffer to store the
    data for the chunck size then add the buffer to the body string
*/

#endif