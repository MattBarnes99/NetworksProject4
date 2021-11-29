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

    // Setup TCP connection
    int sock = setupConnection(serverHost, servPortString);

    // LOGON REQUEST
    u_char logged = 0;
    char username[SHORT_BUFFSIZE];
    char password[SHORT_BUFFSIZE];
    while (!logged) {
        printf("Please enter username: ");
        scanf("%s", username);
        printf("Please enter password: ");
        scanf("%s", password);
        char message[SHORT_BUFFSIZE];
        snprintf(message, strlen(username) + strlen(password) + 2, "%s:%s", username, password);
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


        printf("Please enter a command:\n");
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
        else if (!strcmp(command, "PUSH")){

            // Send Push command for songs that client has that server doesn't
            char filePaths[2][SHORT_BUFFSIZE] = {"Matt_Client/sample1.mp3", "Matt_Client/sample2.mp3"};
            size_t num = 2;
            int fileSizes[(int) num];

            for (size_t i = 0; i < num; i++) {
                int fd = open(filePaths[i], O_RDONLY);
                struct stat file_stat;
                fstat(fd, &file_stat);
                fileSizes[i] = file_stat.st_size;


                char name[SHORT_BUFFSIZE];
                memcpy(name, filePaths[i], SHORT_BUFFSIZE);
                strtok(name, "/");


                char temp[SHORT_BUFFSIZE];
                snprintf(temp, sizeof(filePaths[i]) + sizeof(u_int32_t),
                    "%s:%d:", strtok(NULL, "/"), fileSizes[i]);
                strncat(message, temp, strlen(temp));
            }

            strncat(message, "\n", 1);
            printf("%s\n", message);
            sendPacket(sock, PUSH_TYPE, num, message);

            struct Packet p = receivePacket(sock);

            if (p.type == ACK_TYPE) {
                printf("ready to send files!\n");
                for (size_t i = 0; i < num; i++) {

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

        // LEAVE REQUEST
        else if (!strcmp(command, "LEAVE")){
            printf("leave correct\n");
            sendPacket(sock, LEAVE_TYPE, DEFAULT_LENGTH, DEFAULT_MESSAGE);
            close(sock);
            exit(0); // exit thread later
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