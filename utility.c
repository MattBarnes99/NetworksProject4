#include "utility.h"

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

// Send push packet
void sendPushPacket(char **filePaths, int num, int sock)
{
    int fileSizes[num];
    char message[BUFFSIZE];
    memset(message, 0, sizeof(message));

    for (int i = 0; i < num; i++) {
        int fd = open(filePaths[i], O_RDONLY);
        struct stat file_stat;
        fstat(fd, &file_stat);
        fileSizes[i] = file_stat.st_size;
        // printf("IN SEND PUSH PACKET\n");
        // printf("PATH: %s ; SIZE: %d\n", filePaths[i], fileSizes[i]);


        char name[SHORT_BUFFSIZE];
        memset(name, 0, sizeof(name));
        memcpy(name, filePaths[i], SHORT_BUFFSIZE);
        strtok(name, "/");


        char temp[SHORT_BUFFSIZE];
        memset(temp, 0, sizeof(temp));
        snprintf(temp, sizeof(filePaths[i]) + MAX_DIGIT,
            "%s:%d:", strtok(NULL, "/"), fileSizes[i]);
        strncat(message, temp, strlen(temp));
    }

    strncat(message, "\n", 1);
    printf("%s", message);
    sendPacket(sock, PUSH_TYPE, num, message);
}

// Push files
void pushFiles(char **filePaths, int num, int sock)
{
    struct Packet p = receivePacket(sock);
    if (p.type == ACK_TYPE) {
        printf("ready to send files!\n");
        for (int i = 0; i < num; i++) {
            int fd = open(filePaths[i], O_RDONLY);
            struct stat file_stat;
            fstat(fd, &file_stat);

            off_t offset = 0;
            int sent_bytes = 0;
            int remain_data = file_stat.st_size;

            /* Sending file data */
            while (((sent_bytes = sendfile(sock, fd, &offset, BUFFSIZE)) > 0) && (remain_data > 0)) {
                remain_data -= sent_bytes;
            }

            p = receivePacket(sock);
            if (p.type == ACK_TYPE) {
                printf("File sent successfully!\n");
                continue;
            }
        }
    }
}

char **listDir(char* path, int* numFiles)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(path);
    char** files;

    int num = 0;
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {

            files[num] = dir->d_name;
        }
        closedir(d);
    }

    *numFiles = num;
    return files;
}


void createHash(char *path, char *hash) {
    FILE* mp3file;
    unsigned char md5_digest[MD5_DIGEST_LENGTH];

    mp3file = fopen(path, "rb");

    uint8_t buffer[BUFFERSIZE];

    for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		printf("%02x", (unsigned int) md5_digest[i]);
        char *p = (char *) &md5_digest[i];
        // printf("%s\n",p);
        sprintf(&hash[i*2], "%02x", md5_digest[i]);
        //strncpy(hash, p, strlen(p));
	}
    printf("\n");
    printf("Saved: %s\n", hash);



    int bytes;
    MD5_CTX mdContext;
    MD5_Init (&mdContext);
    while ((bytes = fread (buffer, 1, 1024, mp3file)) != 0)
        MD5_Update (&mdContext, buffer, bytes);
    MD5_Final (md5_digest, &mdContext);
    for(int i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", md5_digest[i]);
    printf (" %s\n", path);
    fclose (mp3file);
}