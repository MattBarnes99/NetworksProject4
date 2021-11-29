#include "utility.h"

#define BUFFERSIZE 1048576
static char *files[100];

// Send packet
size_t sendPacket(int sock, u_char type, u_char length, char *data)
{
    struct Packet p;
    p.type = type;
    p.length = length;
    strncpy(p.data, data, strlen(data) + HEADER_SIZE);

    ssize_t numBytes = send(sock, &p, strlen(p.data) + HEADER_SIZE, 0);
    if (numBytes < 0)
        DieWithError("send() failed");
    else if (numBytes != strlen(p.data) + HEADER_SIZE)
        DieWithError("send() incorrect numbaer of bytes");

    return numBytes;
}

// Receive packet
struct Packet receivePacket(int sock)
{
    struct Packet *p;
    char buffer[BUFFSIZE]; // Buffer for the received message
    unsigned int numBytes = 0;
    unsigned int totalBytes = 0;

    // Receive message from client
    for (;;)
    {
        numBytes = recv(sock, buffer + numBytes, BUFFSIZE - 1, 0);
        if (numBytes < 0)
            DieWithError("recv() failed");
        else if (numBytes == 0)
            DieWithError("recv() connection closed prematurely");
        totalBytes += numBytes;

        // Stop when newline char is received
        if (buffer[totalBytes - 1] == '\n')
        {
            buffer[totalBytes - 1] = '\0';
            break;
        }
    }

    p = (struct Packet *)buffer;
    return *p;
}

// setup TCP connection
int setupConnection(char *serverHost, char *serverPortString)
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        DieWithError("socket() failed");

    // Tell the system what kind(s) of address info we want
    struct addrinfo addrCriteria;                   // Criteria for address match
    memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
    addrCriteria.ai_family = AF_UNSPEC;             // Any address family
    addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
    addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

    // Get address(es) associated with the specified name/service
    struct addrinfo *addrList; // Holder for list of addresses returned

    // Modify servAddr contents to reference linked list of addresses
    int rtnVal = getaddrinfo(serverHost, serverPortString, &addrCriteria, &addrList);
    if (rtnVal != 0)
        DieWithError("getaddrinfo() failed");

    struct addrinfo *addr = addrList;

    // Establish the connection to the server
    if (connect(sock, addr->ai_addr, addr->ai_addrlen) < 0)
        DieWithError("connect() failed");

    return sock;
}

// Die with error message
void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

// Recieve file
unsigned int receiveFile(char *path, int size, int sock)
{
    char message[BUFFSIZE];
    FILE *fptr = fopen(path, "w");
    unsigned int totalBytes = 0;
    unsigned int numBytes = 0;

    printf("Received file: %s of size: %d\n", path, size);
    // Receive message from client
    for (;;)
    {
        numBytes = recv(sock, message, BUFFSIZE, 0);
        totalBytes += numBytes;
        if (numBytes < 0)
            DieWithError("recv() failed");
        else if (numBytes == 0)
            DieWithError("recv() connection closed prematurely");

        fwrite(message, sizeof(char), numBytes, fptr);

        // Stop when newline char is received
        if (totalBytes == size)
            break;
    }

    fclose(fptr);
    return totalBytes;
}

void calculateFileHash(char *path, char *hash) {

    FILE* mp3file;
    unsigned char md5_digest[MD5_DIGEST_LENGTH];

    mp3file = fopen(path, "rb");

    uint8_t buffer[BUFFERSIZE];
	int buff_len = fread(buffer, 1, BUFFERSIZE, mp3file);

    MD5(buffer, buff_len, md5_digest);

    for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(&hash[i*2], "%02x", md5_digest[i]);
	}
    
}

char **listDir(char *path, int numFiles)
{

    memset(files, 0, sizeof files);
    DIR *d;
    struct dirent *dir;
    d = opendir(path);

    int num = 0;
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {

            char *fileName = dir->d_name;

            // Don't count curr dir or back dir
            if (!strcmp(fileName, ".") || !strcmp(fileName, ".."))
            {
                continue;
            }

            files[num] = (char *)malloc(255);
            strncpy(files[num], fileName, 255);

            num++;
        }
        closedir(d);
    }

    return files;
}

int countFilesInDir(char *path)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(path);

    int num = 0;
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            char *fileName = dir->d_name;

            // Don't count curr dir or back dir
            if (!strcmp(fileName, ".") || !strcmp(fileName, ".."))
            {
                continue;
            }

            num++;
        }
        closedir(d);
    }

    return num;
}