/*
    File name: StaticFiles.c
    Created at: 18-12-25
    Author: Solomon
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "StaticFiles.h"
#include "Data Structures/Stack.h"

#define ROOT "./www"

void serverFile(const char* URL)
{
    if (!isSafePath(URL)) return;

    char* filePath = URLToFilePath(URL);
    FileStats fs = getFileStats(filePath);
    printFileStats(&fs);
}

/////////////////////////////////////////
/////////////////////////////////////////
char* URLToFilePath(const char* URL)
{
    if (strcmp(URL, "/") == 0)
        URL = "/index.html";

    size_t rootLen = strlen(ROOT);
    size_t urlLen = strlen(URL);
    size_t bufferSize = 128;

    char* buffer = calloc(bufferSize, 1);
    if (!buffer) return NULL;

    while (rootLen + urlLen + 1 > bufferSize)
    {
        bufferSize *= 2;
        char* temp = realloc(buffer, bufferSize);
        if (!temp)
        {
            free(buffer);
            return NULL;
        }

        buffer = temp;
    }

    strcpy(buffer, ROOT);
    strcat(buffer, URL);

    return buffer;
}

/////////////////////////////////////////
/////////////////////////////////////////
FileStats getFileStats(const char* filePath)
{
    FileStats fs = { 0 };
    struct stat s;

    if (stat(filePath, &s) != 0)
    {
        return fs;
    }

    fs.isRegular = S_ISREG(s.st_mode);
    fs.isDirectory = S_ISDIR(s.st_mode);
    fs.isSymLink = S_ISLNK(s.st_mode);

    fs.canRead = (s.st_mode & S_IRUSR) != 0;
    fs.canWrite = (s.st_mode & S_IWUSR) != 0;
    fs.canExecute = (s.st_mode & S_IXUSR) != 0;

    fs.size = s.st_size;
    fs.owner = s.st_uid;
    fs.group = s.st_gid;

    return fs;
}

////////////////////////////////////////
/////////////////////////////////////////
void printFileStats(const FileStats* fs)
{
    printf("File type:\n");
    printf("  Regular file : %s\n", fs->isRegular ? "yes" : "no");
    printf("  Directory    : %s\n", fs->isDirectory ? "yes" : "no");
    printf("  Symbolic link: %s\n", fs->isSymLink ? "yes" : "no");

    printf("\nPermissions (owner):\n");
    printf("  Read    : %s\n", fs->canRead ? "yes" : "no");
    printf("  Write   : %s\n", fs->canWrite ? "yes" : "no");
    printf("  Execute : %s\n", fs->canExecute ? "yes" : "no");

    printf("\nMetadata:\n");
    printf("  Size    : %ld bytes\n", (long)fs->size);
    printf("  Owner   : UID %u\n", fs->owner);
    printf("  Group   : GID %u\n", fs->group);
}

/////////////////////////////////////////
/////////////////////////////////////////
const char* getMIMEType(const char* filePath)
{
    int dotIndex = -1;
    int len = strlen(filePath);

    for (int x = len - 1; x >= 0; x--)
    {
        if (filePath[x] == '.')
        {
            dotIndex = x;
            break;
        }
    }

    if (dotIndex == -1)
        return "application/octet-stream";

    int extLen = len - dotIndex - 1;
    if (extLen <= 0)
        return "application/octet-stream";

    char* ext = calloc(extLen + 1, 1);
    if (!ext)
        return "application/octet-stream";

    memcpy(ext, filePath + dotIndex + 1, extLen);

    if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0)
    {
        free(ext);
        return "text/html";
    }

    if (strcmp(ext, "css") == 0)
    {
        free(ext);
        return "text/css";
    }

    if (strcmp(ext, "js") == 0)
    {
        free(ext);
        return "application/javascript";
    }

    if (strcmp(ext, "png") == 0)
    {
        free(ext);
        return "image/png";
    }

    if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0)
    {
        free(ext);
        return "image/jpeg";
    }

    if (strcmp(ext, "gif") == 0)
    {
        free(ext);
        return "image/gif";
    }

    if (strcmp(ext, "svg") == 0)
    {
        free(ext);
        return "image/svg+xml";
    }

    if (strcmp(ext, "ico") == 0)
    {
        free(ext);
        return "image/x-icon";
    }

    if (strcmp(ext, "json") == 0)
    {
        free(ext);
        return "application/json";
    }

    if (strcmp(ext, "txt") == 0)
    {
        free(ext);
        return "text/plain";
    }

    if (strcmp(ext, "pdf") == 0)
    {
        free(ext);
        return "application/pdf";
    }

    free(ext);
    return "application/octet-stream";
}

/////////////////////////////////////////
/////////////////////////////////////////
bool isSafePath(const char* URL)
{
    /*
    This function is designed to safely validate and prepare a URL path
    before it is mapped to a filesystem location. Its responsibilities are:

    - Verify the URL pointer is valid and the path is absolute (starts with '/')
    - Reject platform-specific path separators that could bypass checks ('\')
    - Reject ASCII control characters and other non-printable bytes (0x00–0x1F and 0x7F)
    - Decode percent-encoded characters (%XX) exactly once
    - Reject malformed or partial percent-encoding sequences
    - Ensure the decoded path remains well-formed after decoding
    - Normalize the path using stack-based segment processing
        * Ignore "." segments
        * Resolve ".." by popping the previous segment
        * Reject attempts to escape the root directory
        * Reject empty segments caused by "//"
    - Reconstruct a canonical, normalized path string
    - Merge the normalized path with a fixed base directory
    - Resolve the final filesystem path using realpath()
    - Verify the resolved path is contained within the base directory
    - Reject the path if any validation or containment rule is violated

    Only paths that pass all these stages are considered safe for file access.
    */

    //============================================================================//

    // path must start with '/'
    if (URL == NULL) return false;
    if (URL[0] != '/') return false;

    size_t len = strlen(URL);

    char* buffer = (char*)calloc(len + 1, 1);
    if (buffer == NULL) return false;

    size_t i = 0; // index used to fill decoded buffer


    //decode percent-encoded characters + basic checks
    for (size_t x = 0; x < len; x++)
    {
        // windows URL not allowed
        if (URL[x] == '\\') { free(buffer); return false; }

        // reject control characters
        if ((unsigned char)URL[x] < 0x20 || URL[x] == 0x7F)
        {
            free(buffer);
            return false;
        }

        // decode %XX
        if (URL[x] == '%' && (x + 2 < len) &&
            isHex(URL[x + 1]) && isHex(URL[x + 2]))
        {
            unsigned char hi = isHex(URL[x + 1]);
            unsigned char lo = isHex(URL[x + 2]);
            buffer[i++] = (hi << 4) | lo;

            x += 2;
            continue;
        }

        // malformed percent encoding
        if (URL[x] == '%')
        {
            free(buffer);
            return false;
        }

        // fill the buffer
        buffer[i++] = URL[x];
    }

    buffer[i] = '\0';
    size_t bufferLen = i;

    // checks on decoded path (pre-normalization only)
    if (buffer[0] != '/') { free(buffer); return false; }

    for (size_t x = 0; x < bufferLen; x++)
    {
        if (buffer[x] == '\\') { free(buffer); return false; }

        if (buffer[x] == '/' && x + 1 < bufferLen && buffer[x + 1] == '/')
        {
            free(buffer);
            return false;
        }
    }

    /* ------------------------------------------------ */
    /* TODO:
       - Normalize path using stack-based segment handling
       - Reject empty segments (//)
       - Handle "." and ".." correctly
       - Rebuild normalized path
       - Merge with base directory
       - realpath() resolution
       - Verify resolved path stays inside base directory
    */
    /* ------------------------------------------------ */



    free(buffer);
    return true;
}

/////////////////////////////////////////
/////////////////////////////////////////
char* normalizePath(char* path)
{
    Stack stack;
    initialize(&stack, 20, sizeof(char*));

    char* token = strtok(path, "/");

    while (token != NULL)
    {
        if (strcmp(token, ".") == 0)
        {
            token = strtok(NULL, "/");
            continue;
        }

        if (strcmp(token, "..") == 0)
        {
            if (isEmpty(&stack))
                return NULL; // escape root ? reject

            pop(&stack);
            token = strtok(NULL, "/");
            continue;
        }

        push(&stack, token);
        token = strtok(NULL, "/");
    }

    free(path);
    // reconstruction of the path

    char** items = (char**)stack.array;
    size_t total = 1; // leading '/'

    for (size_t x = 0; x < stack.top; x++)
    {
        total += strlen(items[x]) + 1;
    }

    char* buffer = (char*)calloc(total, sizeof(char));
    if (buffer == NULL) return NULL;
    buffer[0] = '/';
    unsigned i = 1;

    for (int x = 0; x < total; x++)
    {
        strcpy(buffer, items[x]);
        i += strlen(x);
        buffer[i++] = '/';
        // to be finished
    }

}

/////////////////////////////////////////
/////////////////////////////////////////
bool isHex(char c)
{
    if (c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f') return true;
    return false;
}

/////////////////////////////////////////
/////////////////////////////////////////
int main()
{
    bool safe = isSafePath("/images/icons/../home.png");
    printf("Safe: %s\n", safe ? "yes" : "no");
    return 0;
}