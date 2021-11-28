#include "utility.h"
#include <stdbool.h>

// Communication with a client
void HandleTCPClient(int clntSocket);

// Receive a message sent
// unsigned int receiveMessage(char* name, int size, int clntSocket);

// size_t sendPacket(int sock, u_char type, u_char length, char *data) {
//     struct Packet p;
//     p.type = type;
//     p.length = length;
//     strncpy(p.data, data, strlen(p.data) + HEADER_SIZE);

//     ssize_t numBytes = send(sock, &p, strlen(p.data) + HEADER_SIZE, 0);
//     if (numBytes < 0)
//         DieWithError("send() failed");
//     else if (numBytes != strlen(p.data) + HEADER_SIZE)
//         DieWithError("send() incorrect numbaer of bytes");

//     return numBytes;
// }

bool validLogon = false;


int main(int argc, char *argv[]) {

    unsigned short serverPort = atoi(SERVER_PORT);

    // Test for correct number of arguments
    if (argc != 3)
        DieWithError("Usage Project4Server -p <port>");

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            char c = argv[i][1];
            switch (c) {
                case 'p':
                    serverPort = htons(atoi(argv[i + 1]));
                    break;
                default:
                    break;
            }
        }
    }

    // Create socket for incoming connections
    int servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (servSock < 0)
        DieWithError("socket() failed");

    // Construct local address structure
    struct sockaddr_in servAddr;                  // Local address
    memset(&servAddr, 0, sizeof(servAddr));       // Zero out structure
    servAddr.sin_family = AF_INET;                // IPv4 address family
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
    servAddr.sin_port = serverPort;               // Local port

    // Bind to the local address
    if (bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");

    // Mark the socket so it will listen for incoming connections
    if (listen(servSock, LISTENQ) < 0)
        DieWithError("listen() failed");

    for (;;) {
        // Client address
        struct sockaddr_in clntAddr;
        // Set length of client address structure (in-out parameter)
        socklen_t clntAddrLen = sizeof(clntAddr);

        // Wait for a client to connect
        int clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
        if (clntSock < 0)
            DieWithError("accept() failed");

        // String to contain client address
        char clntName[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName,
                      sizeof(clntName)) != NULL)
            printf("Handling client %s:%d\n\n", clntName, ntohs(clntAddr.sin_port));
        else
            puts("Unable to get client address");


        // Handle a communication with a client
        HandleTCPClient(clntSock);
    }
}


unsigned int receiveMessage(char* name, int size, int clntSocket) {
    char* message[BUFFSIZE];
    FILE *fptr = fopen(name, "w");
    unsigned int totalBytes = 0;
    unsigned int numBytes = 0;
    // Receive message from client
    for(;;) {
        numBytes = recv(clntSocket, message, BUFFSIZE, 0);
        totalBytes += numBytes;

        printf("Received %d bytes\n", numBytes);
        printf("Total bytes: %d\n", totalBytes);
        if (numBytes < 0)
            DieWithError("recv() failed");
        else if (numBytes == 0)
            DieWithError("recv() connection closed prematurely");

        fwrite(message, sizeof(char), numBytes, fptr);

		// Stop when newline char is received
        // printf("Last bytes: %x ; %x\n", message[totalBytes], message[totalBytes - 1]);
        if(totalBytes == size) {
            // message[totalBytes - 1] = '\0';
            break;
        }
    }

    fclose(fptr);
    printf("Received %d bytes\n", totalBytes);
    return totalBytes;
}


void HandleTCPClient(int clntSocket) {

    for (;;) {
        char buffer[BUFFSIZE]; // Buffer for the received message
        unsigned int numBytes = 0;
        unsigned int totalBytes = 0;

        // Receive message from client
        for(;;) {
            numBytes = recv(clntSocket, buffer + numBytes, BUFFSIZE - 1, 0);
            if (numBytes < 0)
                DieWithError("recv() failed");
            else if (numBytes == 0)
                DieWithError("recv() connection closed prematurely");
            totalBytes += numBytes;
            printf("Received %d bytes\n", numBytes);
            printf("Total: %d bytes\n", totalBytes);
            printf("%s", buffer);

            // Stop when newline char is received
            if(buffer[totalBytes - 1] == '\n') {
                buffer[totalBytes - 1] = '\0';
                break;
            }
        }

        printf("%s", buffer);
        struct Packet* p = (struct Packet*) buffer;
        int type = p->type;

        // LOGON REQUEST
        if (type == LOGON_TYPE){
            char *data = p->data;
            // char *usrName = strtok(info, ":");
            // char *pwd = strtok(NULL, ":");
            FILE *users = fopen("userInfo.txt", "r");

            char line[1024];
            while(fgets(line, 1024, users) ) {
                if (!strcmp(line, data))
                    validLogon = true;
                printf("%s\n", line);
                printf("%d\n", validLogon);
            }

            u_char type = validLogon ? ACK_TYPE : NACK_TYPE;
            sendPacket(clntSocket, type, DEFAULT_LENGTH, "\n");

            fclose(users);
        }

        // LIST REQUEST
        else if (type == 0b00000001){

        }

        // PULL REQUEST
        else if (type == 0b00000011){
            int num = (int) buffer[1];
            int fileSizes[(int) num];
            char fileNames[3][100];

            printf("%d\n", num);
            printf("%s\n", buffer + 2);

            char *name = strtok(buffer + 2, ":");
            char *size = strtok(NULL, ":");
            for (int i = 0; i < num; i++){
                strcpy(fileNames[i], name);
                fileSizes[i] = atoi(size);
                name = strtok(NULL, ":");
                size = strtok(NULL, ":");
            }

            for (int i = 0; i < num; i++){
                receiveMessage(fileNames[i], fileSizes[i], clntSocket);
            }
        }

        // LEAVE REQUEST
        else if (type == 0b00000100){
            close(clntSocket);
        }

        // ELSE INVALID - DO NOTHING
        else{

        }
    }
}

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}