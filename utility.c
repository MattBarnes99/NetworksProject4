#include "utility.h"
static char *files[100];

// Send packet
size_t sendPacket(int sock, u_char type, u_char length, char *data)
{
    // create packet to send
    struct Packet p;
    p.type = type;
    p.length = length;
    strncpy(p.data, data, strlen(data) + HEADER_SIZE);

    // send packet
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
    // create packet to read into from recv
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

    p = (struct Packet *)buffer; // Look at buffer through lens of Packet struct
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
    // new file to create and write to
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

void calculateFileHash(char *path, char *hash)
{

    FILE *mp3file;
    unsigned char md5_digest[MD5_DIGEST_LENGTH];

    mp3file = fopen(path, "rb");

    uint8_t buffer[BUFFERSIZE];
    int buff_len = fread(buffer, 1, BUFFERSIZE, mp3file);

    MD5(buffer, buff_len, md5_digest);

    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        sprintf(&hash[i * 2], "%02x", md5_digest[i]);
    }
}

// Send push packet
void sendPushPacket(char **filePaths, int num, int sock)
{
    int fileSizes[num];
    char message[BUFFSIZE];
    memset(message, 0, sizeof(message));

    // loop to update message to contain name:size pairs of songs to be sent
    for (int i = 0; i < num; i++)
    {
        int fd = open(filePaths[i], O_RDONLY);
        struct stat file_stat; // Get file stats
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
    if (p.type == ACK_TYPE) // recieve ack message in response to push packet before send files
    {
        printf("ready to send files!\n");
        // send each file
        for (int i = 0; i < num; i++)
        {
            int fd = open(filePaths[i], O_RDONLY);
            struct stat file_stat;
            fstat(fd, &file_stat);

            off_t offset = 0;
            int sent_bytes = 0;
            int remain_data = file_stat.st_size;

            /* Sending file data */
            while (((sent_bytes = sendfile(sock, fd, &offset, BUFFSIZE)) > 0) && (remain_data > 0))
            {
                remain_data -= sent_bytes;
            }

            p = receivePacket(sock);
            if (p.type == ACK_TYPE)
            {
                printf("File sent successfully!\n");
                continue;
            }
        }
    }
}

char **listDir(char *path)
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

void createHash(char *path, char *hash)
{ // creates md5 hash
    unsigned char message_digest[MD5_DIGEST_LENGTH];
    int i;
    FILE *mp3file = fopen(path, "rb"); // open the mp3 to be read in binary
    MD5_CTX c;
    int bytes = 0;
    unsigned char data[BUFFERSIZE];

    MD5_Init(&c);
    while ((bytes = fread(data, 1, BUFFERSIZE, mp3file)) != 0) // read mp3file until end
        MD5_Update(&c, data, bytes);                           // continuously hash chunks of data
    MD5_Final(message_digest, &c);                             // place hash in message_digest

    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        sprintf(&hash[i * 2], "%02x", message_digest[i]); // save the hash for the file
    }

    fclose(mp3file); // close file
}

int in_set(char **x, int len, char *match)
{
    int i;
    for (i = 0; i < len; i++)
        if (x[i] && !strcmp(x[i], match))
            return 1;
    return 0;
}

/* x - y */
void get_diff(char **x, int lenx, char **y, int leny, int *ind, int* indArrSize)
{
    int i;
    int c = 0;
    for (i = 0; i < lenx; i++)
        if (x[i] && !in_set(y, leny, x[i]))
        {
            ind[c] = i;
            c++;
        }

    *indArrSize = c;
}

/* X ^ Y */
// void show_sym_diff(char *x[128], int lenx, char *y[128], int leny)
// {
//     show_diff(x, lenx, y, leny);
//     show_diff(y, leny, x, lenx);
// }