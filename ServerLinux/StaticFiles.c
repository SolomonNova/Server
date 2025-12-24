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

bool isSafePath(const char* URL)
{
    if (URL == NULL) return false;
    if (URL[0] != '/') return false;

    unsigned short len = strlen(URL);
    char* buffer = (char*)calloc(len, sizeof(char));
    int i = 0; // the index used to fill the buffer

    for (int x = 0; x < len; x++)
    {
        if (URL[x] == '\\') return false; // windows convention is invalid
        if (URL[x] < 0x20 || URL[x] == 0x7F) return false;

        // decoding endoced characters
        if (URL[x] == '%' && (x + 2 < len) && isHex(URL[x + 1]) && isHex(URL[x + 2]))
        {
            unsigned short encodedDigit = 16 * URL[x + 1] + URL[x + 2];
            buffer[i] = encodedDigit;
            x += 3;
            continue;
        }

        // malformed encoding
        if (URL[x] == '%') return false;

        i++;
        buffer[i] = URL[x];

    }

    if (buffer == NULL) return false;
    if (buffer[0] != '/') return false;
    unsigned short bufferLen = strlen(buffer);


    for (int x = 0; x < bufferLen; x++)
    {
        if (buffer[x] == '\\') return false;
        if (buffer[x] + 2 < bufferLen && buffer[i] == '.' && buffer[i + 1] == '.' && buffer[i + 2] == '/') return false;
    }

    // to be finished, a stack is needed to normalize the URL path

}

/////////////////////////////////////////
/////////////////////////////////////////
bool isHex(char c)
{
    if (c >= '0' && c <= '9' || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f') return true;
    return false;
}