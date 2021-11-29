#include "utility.h"
bool validLogon = false;

// Communication with a client
void HandleTCPClient(int clntSocket, char* username);
void handleListRequest(struct Packet p);


void authorize(int clntSocket, char *username) {
    for (;;) {
        // Receive message from client
        struct Packet p = receivePacket(clntSocket);
        char *data = p.data;
        FILE *users = fopen("userInfo.txt", "r");

        char line[1024];
        while(fgets(line, 1024, users) ) {
            if (!strcmp(line, data))
                validLogon = true;
                memcpy(username, strtok(line, ":"), SHORT_BUFFSIZE);
        }

        u_char type = validLogon ? ACK_TYPE : NACK_TYPE;
        sendPacket(clntSocket, type, DEFAULT_LENGTH, "\n");
        fclose(users);
        if (validLogon)
            break;
    }
}


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
        char username[SHORT_BUFFSIZE];
        authorize(clntSock, username);
        printf("HERE\n");
        printf("USERNAME: %s\n", username);
        HandleTCPClient(clntSock, username);
    }
}


// unsigned int receiveFile(char* name, int size, int clntSocket) {
//     char* message[BUFFSIZE];
//     FILE *fptr = fopen(name, "w");
//     unsigned int totalBytes = 0;
//     unsigned int numBytes = 0;
//     // Receive message from client
//     for(;;) {
//         numBytes = recv(clntSocket, message, BUFFSIZE, 0);
//         totalBytes += numBytes;

//         printf("Received %d bytes\n", numBytes);
//         printf("Total bytes: %d\n", totalBytes);
//         if (numBytes < 0)
//             DieWithError("recv() failed");
//         else if (numBytes == 0)
//             DieWithError("recv() connection closed prematurely");

//         fwrite(message, sizeof(char), numBytes, fptr);

// 		// Stop when newline char is received
//         // printf("Last bytes: %x ; %x\n", message[totalBytes], message[totalBytes - 1]);
//         if(totalBytes == size) {
//             // message[totalBytes - 1] = '\0';
//             break;
//         }
//     }

//     fclose(fptr);
//     printf("Received %d bytes\n", totalBytes);
//     return totalBytes;
// }


void HandleTCPClient(int clntSocket, char* username) {

    for (;;) {

        // Receive message from client
        struct Packet p = receivePacket(clntSocket);
        int type = p.type;

        // LIST REQUEST
        if (type == LIST_TYPE) {
            printf("LIST REQUEST\n");
            // handleListRequest(p);
        }

        // PULL REQUEST
        else if (type == PULL_TYPE) {
            printf("PULL REQUEST\n");
            // int num = p.length;
            // int fileSizes[(int) num];
            // char fileNames[3][100];

            // printf("%d\n", num);
            // printf("%s\n", buffer + 2);

            // char *name = strtok(buffer + 2, ":");
            // char *size = strtok(NULL, ":");
            // for (int i = 0; i < num; i++){
            //     strcpy(fileNames[i], name);
            //     fileSizes[i] = atoi(size);
            //     name = strtok(NULL, ":");
            //     size = strtok(NULL, ":");
            // }

            // for (int i = 0; i < num; i++){
            //     receiveMessage(fileNames[i], fileSizes[i], clntSocket);
            // }
        }

        // PUSH REQUEST
        else if (type == PUSH_TYPE) {
            printf("PUSH REQUEST\n");
            int num = p.length;
            int fileSizes[(int) num];
            char fileNames[num][SHORT_BUFFSIZE];

            char *name = strtok(p.data, ":");
            char *size = strtok(NULL, ":");
            for (int i = 0; i < num; i++){
                strcpy(fileNames[i], name);
                fileSizes[i] = atoi(size);
                name = strtok(NULL, ":");
                size = strtok(NULL, ":");
            }

            printf("Parsed correctly\n");
            sendPacket(clntSocket, ACK_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);

            for (int i = 0; i < num; i++){
                char path[SHORT_BUFFSIZE];
                snprintf(path, sizeof(path), "%s_Server/%s", username, fileNames[i]);
                receiveFile(path, fileSizes[i], clntSocket);
                sendPacket(clntSocket, ACK_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);
            }
        }

        // LEAVE REQUEST
        else if (type == LEAVE_TYPE) {
            printf("LEAVE REQUEST\n");
            close(clntSocket);
            // Kill thread
        }


        // ELSE INVALID - DO NOTHING
        else
            printf("INVALID REQUEST\n");
    }
}

void handleListRequest(struct Packet p) {

    // 1. Get all files for current user
    // char* user = "Jack";

    // int* numFiles;
    // char** files[100][100];
    // files = listDir(user, numFiles);
    // printf("Num files: %d", *numFiles);

    // for(int i = 0; i < *numFiles; i++) {
    //     printf("%s", *(files+i));
    // }



    // 2. Hash files and put them in a list
    // 3. Format response LIST packet
    // 4. Send LIST response

}