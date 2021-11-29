#include "utility.h"
bool validLogon = false;

// Communication with a client
void HandleTCPClient(int clntSocket, char* username);
void handleListRequest(struct Packet p);
void *ThreadMain(void *arg); // Main program of a thread
void authorize(int clntSocket, char *username);

struct ThreadArgs { // for threading
    int clntSock; // Socket descriptor for client
};


void *ThreadMain(void *threadArgs) {
    // Guarantees that thread resources are deallocated upon return
    pthread_detach(pthread_self());

    // Extract socket file descriptor from argument
    int clntSock = ((struct ThreadArgs *) threadArgs)->clntSock;
    free(threadArgs); // Deallocate memory for argument

    char *username = malloc(SHORT_BUFFSIZE);

    authorize(clntSock, username); // verify login information
    printf("Logged in user: %s\n", username);
    HandleTCPClient(clntSock, username); // handles client
    free(username); // dealocate username memory
    printf("Closing thread %lx\n", (long int) pthread_self());
    return (NULL);
}


void authorize(int clntSocket, char *username) {
    for (;;) {
        // Receive message from client
        struct Packet p = receivePacket(clntSocket);
        char *data = p.data;
        FILE *users = fopen("userInfo.txt", "r");

        char line[1024];
        while(fgets(line, 1024, users) ) {

            // remove newline from fgets
            line[strcspn(line, "\n")] = '\0';

            if (!strcmp(line, data)){
                validLogon = true;
                memcpy(username, strtok(line, ":"), SHORT_BUFFSIZE);
            }
        }
        
        // send ACK or NACK back depending on if logon was valid
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
            printf("\nHandling client %s:%d\n", clntName, ntohs(clntAddr.sin_port));
        else
            puts("Unable to get client address");


        // Threading
        // Create separate memory for client argument
        struct ThreadArgs *threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs));
        if (threadArgs == NULL)
            DieWithError("malloc() failed");
        threadArgs->clntSock = clntSock;

        // Create client thread
        pthread_t threadID;
        int returnValue = pthread_create(&threadID, NULL, ThreadMain, threadArgs);
        if (returnValue != 0)
            DieWithError("pthread_create() failed");
        printf("With thread %lx\n", (long int) threadID);
    }
}

// handles client requests
void HandleTCPClient(int clntSocket, char* username) {

    for (;;) {

        // Receive message from client
        struct Packet p = receivePacket(clntSocket);
        int type = p.type;

        // LIST REQUEST
        if (type == LIST_TYPE) {
            printf("%s; LIST REQUEST\n", username);
            // handleListRequest(p);
        }

        // PULL REQUEST
        else if (type == PULL_TYPE) {
            printf("%s; PULL REQUEST\n", username);
            int num = p.length;
            char *filePaths[num];

            char *temp = strtok(p.data, ":");
            for (int i = 0; i < num; i++) {
                char path[SHORT_BUFFSIZE];
                memset(path, 0, sizeof(path));
                snprintf(path, sizeof(path), "%s_Server/%s", username, temp);
                filePaths[i] = malloc(sizeof(path));
                memcpy(filePaths[i], path, SHORT_BUFFSIZE);
                temp = strtok(NULL, ":");
            }

            sendPushPacket(filePaths, num, clntSocket);
            pushFiles(filePaths, num, clntSocket);

            for (size_t i = 0; i < num; i++)
                free(filePaths[i]);
        }

        // PUSH REQUEST
        else if (type == PUSH_TYPE) {
            printf("%s; PUSH REQUEST\n", username);
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
            printf("%s; LEAVE REQUEST\n", username);
            close(clntSocket);
            return;
        }


        // ELSE INVALID - DO NOTHING
        else
            printf("%s; INVALID REQUEST\n", username);
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