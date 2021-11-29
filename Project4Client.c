#include "utility.h"

int main(int argc, char *argv[]) {

    char *serverHost = SERVER_HOST;
    char *servPortString = SERVER_PORT;
    char command[7];
    struct Packet p;

    // Test for correct number of arguments
    if (argc != 5)
        DieWithError("Usage Project4Client -h <IP/hostname> -p <port>");

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            char c = argv[i][1];
            switch (c) {
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
    while (c != EOF){
        printf ("%c", c);
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
    while (!logged) {
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
        if (p.type == ACK_TYPE) {
            logged = 1;
            printf("Logon successful!\n");
        }

        // Logon Unsuccessful
        else if (p.type == NACK_TYPE) {
            printf("Logon unsuccessful! Try again\n");
            continue;
        }
    }

    for (;;) {

        char message[BUFFSIZE];
        memset(message, 0, sizeof(message));

        // Ask user to enter a command
        printf("\nPlease enter a command (type help for list of commands):\n");
        scanf("%s", command);

        // Convert command to uppercase
        char* str = command;
        while (*str) {
            *str = toupper((unsigned char) *str);
            str++;
        }

        // LIST REQUEST
        if (!strcmp(command, "LIST")){
            sendPacket(sock, LIST_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);
            p = receivePacket(sock);
            printf("%s's files on the server:\n", username);
            char *name = strtok(p.data, ":");
            for (int i = 0; i < p.length; i++){
                printf("%s\n", name);
                strtok(NULL, ":");
                name = strtok(NULL, ":");
            }
        }

        // DIFF REQUEST
        else if (!strcmp(command, "DIFF")){
            sendPacket(sock, LIST_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);
            p = receivePacket(sock);

            char hash[128];
            char path[SHORT_BUFFSIZE];
            memset(path, 0, sizeof(path));
            snprintf(path, sizeof(path), "%s_Client", username);

            char **filesClient;
            filesClient = listDir(path);
            int numFilesClient = countFilesInDir(path);
            

            char clientFilesString[BUFFSIZE];
            memset(clientFilesString, 0, sizeof(clientFilesString));
            for (int i = 0; i < numFilesClient; i++) {
                memset(hash, 0, sizeof hash);
                char filePath[sizeof(path) + sizeof(filesClient[i]) + 1];
                sprintf(filePath, "%s/%s", path, filesClient[i]);
                calculateFileHash(filePath, hash);

                char temp[SHORT_BUFFSIZE];
                snprintf(temp, strlen(filesClient[i]) + MAX_DIGIT,
                    "%s:%s:", filesClient[i], hash);
                strncat(clientFilesString, temp, strlen(temp));
            }
            strncat(clientFilesString, "\n", 1);
            printf("%s", clientFilesString);

            char clientHasServerDont[SHORT_BUFFSIZE][SHORT_BUFFSIZE];
            char serverHasClientDont[SHORT_BUFFSIZE][SHORT_BUFFSIZE];

            char *saveClient, *saveServer;

            char *nameClient = strtok_r(clientFilesString, ":", &saveClient);
            char *hashClient = strtok_r(NULL, ":", &saveClient);


            strtok_r(p.data, ":", &saveServer);
            char *hashServer = strtok_r(NULL, ":", &saveServer);

            int same = 0;
            int numClientHas = 0;

            for (int i = 0; i < numFilesClient; i++) {
                same = 0;
                for (int j = 0; j < p.length; j++){
                    //comparison
                    if(!strcmp(hashClient, hashServer)) {
                        same = 1;
                        printf("%s\n", hashClient);
                        break;
                    }
                    //update server
                    strtok_r(NULL, ":", &saveServer);
                    hashServer = strtok_r(NULL, ":", &saveServer);
                }
                if (!same) {
                    printf("before");
                    memcpy(clientHasServerDont[numClientHas], nameClient, strlen(nameClient));
                    printf("after");
                    numClientHas++;
                }

                //update client
                nameClient = strtok_r(NULL, ":", &saveClient);
                hashClient = strtok_r(NULL, ":", &saveClient);
            }

            printf("Files on the client not server:\n");
            for (int i = 0; i < numClientHas; i++) {
                printf("%s\n", clientHasServerDont[i]);
            }




            // // Get list of files on client
            // filesClient = listDir(path);
            // int numFiles = countFilesInDir(path);

            // printf("%s's files on the server:\n", username);
            // for (int i = 0; i < p.length; i++)
            // {
            //     printf("%s; ", filesServer[i]);
            // }
            // printf("\n");
            // printf("%s's files on the client:\n", username);
            // for (int i = 0; i < p.length; i++)
            // {
            //     printf("%s; ", filesClient[i]);
            // }

            // printf("\n");
            


        }

        // PULL (SYNC) REQUEST
        else if (!strcmp(command, "PULL")){

            // GET FILE NAMES FOR PUSH AND PULL FROM DIFF
            // SEND PUSH PACKET
            // PUSH FILES TO SERVER
            // SEND PULL PACKET
            // RECEIVE FILES FROM SERVER

            char *filePaths[2] = {"Matt_Client/sample1.mp3", "Matt_Client/sample2.mp3"}; 
            int num = 2;
            sendPushPacket(filePaths, num, sock);
            pushFiles(filePaths, num, sock);

            // Sending pull request
            char *pullFiles[2] = {"sample3.mp3", "sample4.mp3"};
            num = 2;
            char message[BUFFSIZE];
            memset(message, 0, sizeof(message));
            for (int i = 0; i < num; i++) {
                char temp[SHORT_BUFFSIZE];
                snprintf(temp, sizeof(filePaths[i]) + MAX_DIGIT,
                    "%s:", pullFiles[i]);
                strncat(message, temp, strlen(temp));
            }
            strncat(message, "\n", 1);
            printf("%s", message);
            sendPacket(sock, PULL_TYPE, num, message);

            // Accepting that ready to get files
            p = receivePacket(sock);
            int fileSizes[num];
            char fileNames[num][SHORT_BUFFSIZE];

            char *name = strtok(p.data, ":");
            char *size = strtok(NULL, ":");
            for (int i = 0; i < num; i++){
                strcpy(fileNames[i], name);
                fileSizes[i] = atoi(size);
                name = strtok(NULL, ":");
                size = strtok(NULL, ":");
            }

            sendPacket(sock, ACK_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);

            // RECEIVE FILES FROM SERVER
            for (int i = 0; i < num; i++){
                char path[SHORT_BUFFSIZE];
                snprintf(path, sizeof(path), "%s_Client/%s", username, pullFiles[i]);
                receiveFile(path, fileSizes[i], sock);
                sendPacket(sock, ACK_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);
            }
        }

        // LEAVE REQUEST
        else if (!strcmp(command, "LEAVE")){
            printf("\nGood Bye!\n");
            sendPacket(sock, LEAVE_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);
            close(sock);
            break;
        }

         // HELP REQUEST
        else if (!strcmp(command, "HELP")){
            printf("COMMANDS:\n");
            printf("LIST:   Lists all songs that the server has stored for client\n");
            printf("DIFF:   Lists songs that client has that server does not AND songs that server has that client does not\n");
            printf("PULL:   Synchronizes files so that server and client will have same songs\n");
            printf("LEAVE:  Ends session for user\n");
        }

        // INVALID COMMAND
        else{
            printf("Invalid input, please input command again\n");
            printf("Valid commands are: LIST, DIFF, PULL, and LEAVE\n");
        }
    }
    return 0;
}