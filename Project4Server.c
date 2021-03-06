#include "utility.h"


bool validLogon = false;

// Communication with a client
void HandleTCPClient(int clntSocket, char* username);
void handleListRequest(char* username, struct Packet p, int clntSocket);
void *ThreadMain(void *arg); // Main program of a thread
void authorize(int clntSocket, char *username);

// for threading
struct ThreadArgs{
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

        // hashing the user password
        char *user = strtok(data,":");
        char *pwd = strtok(NULL,":");
        char pwd_hash[128];
        unsigned char message_digest[MD5_DIGEST_LENGTH];
        MD5_CTX c;
        MD5_Init(&c);
        MD5_Update(&c, pwd, strlen(pwd));   // continuously hash chunks of data
        MD5_Final(message_digest, &c); // place hash in message_digest
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            sprintf(&pwd_hash[i * 2], "%02x", message_digest[i]); // save the hash for the file
        }

        //concatenate user input to username:hashed_pwd for comparison
        char user_input[SHORT_BUFFSIZE];
        memset(user_input, 0, sizeof(user_input));
        snprintf(user_input, sizeof(user_input), "%s:%s", user, pwd_hash);

        char line[1024];
        // look at all users that we have on file
        while(fgets(line, 1024, users) ) {

            // remove newline from fgets
            line[strcspn(line, "\n")] = '\0';

            if (!strcmp(line, user_input)) {
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
        int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
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
            printf("\n%s; LIST REQUEST\n", username);
            handleListRequest(username, p, clntSocket);
        }

        // PULL REQUEST
        else if (type == PULL_TYPE) {
            printf("\n%s; PULL REQUEST\n", username);
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
            printf("\n%s; PUSH REQUEST\n", username);
            int num = p.length; // number of songs to be pushed
            int fileSizes[(int) num]; // associated file sizes
            char fileNames[num][SHORT_BUFFSIZE]; // song names

            // parse the message to get file names and sizes
            char *name = strtok(p.data, ":");
            char *size = strtok(NULL, ":");
            for (int i = 0; i < num; i++){
                strcpy(fileNames[i], name);
                fileSizes[i] = atoi(size);
                name = strtok(NULL, ":");
                size = strtok(NULL, ":");
            }

            // acknowledge that a push request was received...
            sendPacket(clntSocket, ACK_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);

            // ...receive the songs one by one
            for (int i = 0; i < num; i++){
                char path[SHORT_BUFFSIZE];
                snprintf(path, sizeof(path), "%s_Server/%s", username, fileNames[i]);
                receiveFile(path, fileSizes[i], clntSocket);
                sendPacket(clntSocket, ACK_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);
            }
        }

        // LEAVE REQUEST
        else if (type == LEAVE_TYPE) {
            printf("\n%s; LEAVE REQUEST\n", username);
            close(clntSocket);
            return;
        }


        // ELSE INVALID - DO NOTHING
        else
            printf("\n%s; INVALID REQUEST\n", username);
    }
}

void handleListRequest(char *username, struct Packet p, int clntSocket) {

    // 1. Get all files for current user
    // When this is threaded we'll need to get the username from a global in the thread
    // and decide folder name that way.
    char path[SHORT_BUFFSIZE];
    memset(path, 0, sizeof(path));
    snprintf(path, sizeof(path), "%s_Server", username);

    // get number of files in dir
    int numFiles = countFilesInDir(path);

    // printf("%d files in directory.\n", numFiles);

    char* files[255];
    listDir(path, files);
    char hash[128];

    char message[BUFFSIZE];
    memset(message, 0, sizeof(message));
    for (int i = 0; i < numFiles; i++) {

        memset(hash, 0, sizeof hash);
        char filePath[strlen(path) + strlen(files[i]) + 1];
        sprintf(filePath, "%s/%s", path, files[i]);
        createHash(filePath, hash);

        char temp[SHORT_BUFFSIZE];
        snprintf(temp, strlen(files[i]) + MAX_DIGIT,
            "%s:%s:", files[i], hash);
        strncat(message, temp, strlen(temp));
    }
    strncat(message, "\n", 1);
    sendPacket(clntSocket, LIST_TYPE, numFiles, message);

}