#include "utility.h"
void getDiffInfo(int sock, char *username, int *onlyOnServerInd, int *onlyOnClientInd, int *serverIndArrSize, int *clientIndArrSize, char **clientFiles, char *serverFiles[255]);

int main(int argc, char *argv[])
{

    char *serverHost = SERVER_HOST;
    char *servPortString = SERVER_PORT;
    char command[7];
    struct Packet p;

    // Test for correct number of arguments
    if (argc != 5)
        DieWithError("Usage Project4Client -h <IP/hostname> -p <port>");

    // Parse arguments
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            char c = argv[i][1];
            switch (c)
            {
            case 'h':
                serverHost = argv[i + 1];
                break;
            case 'p':
                servPortString = argv[i + 1];
                break;
            // case 'f':
            //     fileName = argv[i + 1];
            //     break;
            default:
                break;
            }
        }
    }

    // Print banner
    FILE *banner = fopen("name.txt", "r");
    char c = fgetc(banner);
    while (c != EOF)
    {
        printf("%c", c);
        c = fgetc(banner);
    }
    fclose(banner);
    printf("\n");

    // Setup TCP connection
    int sock = setupConnection(serverHost, servPortString);

    // LOGON REQUEST
    u_char logged = 0;
    char username[SHORT_BUFFSIZE];
    char password[SHORT_BUFFSIZE];
    while (!logged)
    {
        // Enter credentials
        printf("Please enter username: ");
        scanf("%s", username);
        printf("Please enter password: ");
        scanf("%s", password);

        // Prepare to send packet with login info
        char message[SHORT_BUFFSIZE];
        snprintf(message, strlen(username) + strlen(password) + HEADER_SIZE,
                 "%s:%s", username, password);
        strncat(message, "\n", 1);

        sendPacket(sock, LOGON_TYPE, DEFAULT_LENGTH, message);

        struct Packet p = receivePacket(sock);

        // Logon Successful
        if (p.type == ACK_TYPE)
        {
            logged = 1;
            printf("Logon successful!\n");
        }

        // Logon Unsuccessful
        else if (p.type == NACK_TYPE)
        {
            printf("Logon unsuccessful! Try again\n");
            continue;
        }
    }

    for (;;)
    {

        char message[BUFFSIZE];
        memset(message, 0, sizeof(message));

        // Ask user to enter a command
        printf("\nPlease enter a command (type help for list of commands):\n");
        scanf("%s", command);

        // Convert command to uppercase
        char *str = command;
        while (*str)
        {
            *str = toupper((unsigned char)*str);
            str++;
        }

        // LIST REQUEST
        if (!strcmp(command, "LIST"))
        {
            sendPacket(sock, LIST_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);
            p = receivePacket(sock);
            printf("%s's files on the server:\n", username);
            char *name = strtok(p.data, ":");
            for (int i = 0; i < p.length; i++)
            {
                printf("%s\n", name);
                strtok(NULL, ":");
                name = strtok(NULL, ":");
            }
        }

        // DIFF REQUEST
        else if (!strcmp(command, "DIFF"))
        {

            printf("\n");

            char *serverFiles[255];
            char *clientFiles[255];
            int serverIndArrSize = 0;
            int clientIndArrSize = 0;
            int onlyOnServerInd[sizeof(int)] = {0};
            int onlyOnClientInd[sizeof(int)] = {0};

            getDiffInfo(sock, username, onlyOnServerInd, onlyOnClientInd, &serverIndArrSize, &clientIndArrSize, clientFiles, serverFiles);

            if (serverIndArrSize == 0 && clientIndArrSize == 0)
            {
                printf("The server and client are in sync.\n");
            }

            if (serverIndArrSize > 0)
            {
                printf("Files on server that are not on client:\n");
                for (int i = 0; i < serverIndArrSize; i++)
                {
                    printf("- %s\n", serverFiles[onlyOnServerInd[i]]);
                }
            }

            if (clientIndArrSize > 0)
            {
                printf("Files on client that are not on server:\n");
                for (int i = 0; i < clientIndArrSize; i++)
                {
                    printf("- %s\n", clientFiles[onlyOnClientInd[i]]);
                }
            }
        }

        // PULL (SYNC) REQUEST
        else if (!strcmp(command, "PULL"))
        {

            // GET FILE NAMES FOR PUSH AND PULL FROM DIFF
            // SEND PUSH PACKET
            // PUSH FILES TO SERVER
            // SEND PULL PACKET
            // RECEIVE FILES FROM SERVER

            char *serverFiles[255];
            char *clientFiles[255];
            int serverIndArrSize = 0;
            int clientIndArrSize = 0;
            int onlyOnServerInd[sizeof(int)] = {0};
            int onlyOnClientInd[sizeof(int)] = {0};
            char path[strlen(username) + strlen("_Client")];
            sprintf(path, "%s%s", username, "_Client");

            getDiffInfo(sock, username, onlyOnServerInd, onlyOnClientInd, &serverIndArrSize, &clientIndArrSize, clientFiles, serverFiles);

            printf("\n");

            if (clientIndArrSize > 0)
            {
                printf("Pushing the following files to the server...\n");
            }
            else
            {
                printf("No files to push to server.\n");
            }

            char *filePaths[clientIndArrSize];
            // Files on client that are not on server
            for (int i = 0; i < clientIndArrSize; i++)
            {
                sprintf(filePaths[i], "%s/%s", path, clientFiles[onlyOnClientInd[i]]);
                // printf("- %s\n", clientFiles[onlyOnClientInd[i]]);
                printf("- %s\n", filePaths[i]);
            }

            printf("\n");
            if (serverIndArrSize > 0)
            {
                printf("Pulling the following files from the server...\n");
            }
            else
            {
                printf("No files to pull from server.\n");
            }

            char pullFiles[serverIndArrSize][SHORT_BUFFSIZE];
            // Files on server that are not on client
            for (int i = 0; i < serverIndArrSize; i++)
            {
                sprintf(pullFiles[i], "%s", serverFiles[onlyOnServerInd[i]]);
                printf("- %s\n", pullFiles[i]);
            }
            printf("\n");

            sendPushPacket(filePaths, clientIndArrSize, sock);
            pushFiles(filePaths, clientIndArrSize, sock);

            // Sending pull request
            char message[BUFFSIZE];
            memset(message, 0, sizeof(message));
            for (int i = 0; i < serverIndArrSize; i++)
            {
                char temp[SHORT_BUFFSIZE];
                snprintf(temp, sizeof(filePaths[i]) + MAX_DIGIT,
                         "%s:", pullFiles[i]);
                strncat(message, temp, strlen(temp));
            }
            strncat(message, "\n", 1);
            printf("%s", message);
            sendPacket(sock, PULL_TYPE, serverIndArrSize, message);

            // Accepting that ready to get files
            p = receivePacket(sock);
            int fileSizes[serverIndArrSize];
            char fileNames[serverIndArrSize][SHORT_BUFFSIZE];

            char *name = strtok(p.data, ":");
            char *size = strtok(NULL, ":");
            for (int i = 0; i < serverIndArrSize; i++)
            {
                strcpy(fileNames[i], name);
                fileSizes[i] = atoi(size);
                name = strtok(NULL, ":");
                size = strtok(NULL, ":");
            }

            sendPacket(sock, ACK_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);

            // RECEIVE FILES FROM SERVER
            for (int i = 0; i < serverIndArrSize; i++)
            {
                char path[SHORT_BUFFSIZE];
                snprintf(path, sizeof(path), "%s_Client/%s", username, pullFiles[i]);
                receiveFile(path, fileSizes[i], sock);
                sendPacket(sock, ACK_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);
            }
        }

        // LEAVE REQUEST
        else if (!strcmp(command, "LEAVE"))
        {
            printf("\nGood Bye!\n");
            sendPacket(sock, LEAVE_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);
            close(sock);
            break;
        }

        // HELP REQUEST
        else if (!strcmp(command, "HELP"))
        {
            printf("COMMANDS:\n");
            printf("LIST:   Lists all songs that the server has stored for client\n");
            printf("DIFF:   Lists songs that client has that server does not AND songs that server has that client does not\n");
            printf("PULL:   Synchronizes files so that server and client will have same songs\n");
            printf("LEAVE:  Ends session for user\n");
        }

        // INVALID COMMAND
        else
        {
            printf("Invalid input, please input command again\n");
            printf("Valid commands are: LIST, DIFF, PULL, and LEAVE\n");
        }
    }
    return 0;
}

void getDiffInfo(int sock, char *username, int *onlyOnServerInd, int *onlyOnClientInd, int *serverIndArrSize, int *clientIndArrSize, char **clientFiles, char **serverFiles)
{
    // Get list data from server
    sendPacket(sock, LIST_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);
    struct Packet p = receivePacket(sock);

    // Create path to local files from username
    char path[strlen(username) + strlen("_Client")];
    sprintf(path, "./%s%s", username, "_Client");

    // Get number of files in dir
    int numFiles = countFilesInDir(path);

    // Hash local files
    listDir(path, clientFiles);
    char *clientHashes[128];
    memset(clientHashes, 0, sizeof(clientHashes));
    char hash[128];
    char message[BUFFSIZE];
    memset(message, 0, sizeof(message));
    for (int i = 0; i < numFiles; i++)
    {
        // Clear hash variable
        memset(hash, 0, sizeof hash);

        // Build full path to file
        char filePath[strlen(path) + strlen(clientFiles[i]) + 1];
        sprintf(filePath, "%s/%s", path, clientFiles[i]);
        // printf("%s\n", filePath);

        // Compute hash and save into hash variable
        calculateFileHash(filePath, hash);

        // strncpy(clientHashes[i], hash, strlen(hash));
        clientHashes[i] = strndup(hash, strlen(hash));
    }

    // Parse server hashes / files
    char *serverHashes[128];
    memset(serverFiles, 0, sizeof(*serverFiles));
    memset(serverHashes, 0, sizeof(serverHashes));

    // Context for strtok
    char *context = NULL;

    char *serverFile = strtok_r(p.data, ":", &context);
    char *serverHash = strtok_r(NULL, ":", &context);

    int count = 0;
    while (serverHash != NULL)
    {
        serverFiles[count] = serverFile;
        serverHashes[count] = strndup(serverHash, strlen(serverHash));
        count++;

        serverFile = strtok_r(NULL, ":", &context);
        serverHash = strtok_r(NULL, ":", &context);
    }

    // printf("Server Hashes:\n");
    // for (int i = 0; i < count; i++)
    // {
    //     printf("%s -> %s\n", serverFiles[i], serverHashes[i]);
    // }

    // Server - Client
    get_diff(serverHashes, count, clientHashes, numFiles, onlyOnServerInd, serverIndArrSize);

    // Client - Server
    get_diff(clientHashes, numFiles, serverHashes, count, onlyOnClientInd, clientIndArrSize);
}