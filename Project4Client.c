#include "utility.h"

int main(int argc, char *argv[]) {

    char *serverHost = SERVER_HOST;
    char *servPortString = SERVER_PORT;
    char command[7];
    // char *fileName;

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
        char* str = command;
        while (*str) {
            *str = toupper((unsigned char) *str);
            str++;
        }

        // LIST REQUEST
        if (!strcmp(command, "LIST")){
            printf("list correct\n");
            sendPacket(sock, LIST_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);
        }

        // DIFF REQUEST
        else if (!strcmp(command, "DIFF")){
            printf("diff correct\n");
            sendPacket(sock, LIST_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);
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
            struct Packet p = receivePacket(sock);
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
            printf("leave correct\n");
            sendPacket(sock, LEAVE_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);
            close(sock);
            exit(0); // exit thread later
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


    // size_t num = 3;
    // char fileNames[3][100] = {"sample1.mp3", "sample2.mp3", "sample3.mp3"};
    // int fileSizes[(int) num];
    // struct Packet sending;
    // sending.type = 0b00000010;
    // sending.length = (u_char) num;

    // for (size_t i = 0; i < num; i++) {
    //     int fd = open(fileNames[i], O_RDONLY);
    //     struct stat file_stat;
    //     fstat(fd, &file_stat);
    //     fileSizes[i] = file_stat.st_size;
    //     char temp[SHORT_BUFFSIZE];
    //     snprintf(temp, sizeof(fileNames[i])+32, "%s:%d:", fileNames[i], fileSizes[i]);
    //     strncat(sending.data, temp, strlen(temp));
    // }
    // strncat(sending.data, "\n", 1);
    // printf("%s\n", sending.data);
    // printf("%d", send(sock, &sending, strlen(sending.data) + 2, 0));

    // for (size_t i = 0; i < num; i++) {

    //     int fd = open(fileNames[i], O_RDONLY);
    //     struct stat file_stat;
    //     fstat(fd, &file_stat);
    //     printf("File: %s ; File size: %zu\n", fileNames[i], file_stat.st_size);

    //     off_t offset = 0;
    //     int sent_bytes = 0;
    //     int remain_data = file_stat.st_size;

    //     /* Sending file data */
    //     while (((sent_bytes = sendfile(sock, fd, &offset, BUFFSIZE)) > 0) && (remain_data > 0)) {
    //             printf("Sent: %d ; Offset: %ld ; Remaining: %d\n", sent_bytes, offset, remain_data);
    //             remain_data -= sent_bytes;
    //     }
    //     sleep(2);
    // }

    for(;;) {
        sleep(1);
    }
    return 0;
}