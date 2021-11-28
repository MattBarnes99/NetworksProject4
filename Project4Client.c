#include "utility.h"
#include <ctype.h>


void DieWithError(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

// int setupConnection(char *serverHost, char *serverPortString) {
//     int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//     if (sock < 0)
//         DieWithError("socket() failed");

//     // Tell the system what kind(s) of address info we want
//     struct addrinfo addrCriteria;                   // Criteria for address match
//     memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
//     addrCriteria.ai_family = AF_UNSPEC;             // Any address family
//     addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
//     addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

//     // Get address(es) associated with the specified name/service
//     struct addrinfo *addrList; // Holder for list of addresses returned

//     // Modify servAddr contents to reference linked list of addresses
//     int rtnVal = getaddrinfo(serverHost, serverPortString, &addrCriteria, &addrList);
//     if (rtnVal != 0)
//         DieWithError("getaddrinfo() failed");

//     struct addrinfo *addr = addrList;

//     // Establish the connection to the server
//     if (connect(sock, addr->ai_addr, addr->ai_addrlen) < 0)
//         DieWithError("connect() failed");

//     return sock;
// }

// size_t sendPacket(int sock, u_char type, u_char length, char *data) {
//     struct Packet p;
//     p.type = type;
//     p.length = length;
//     strncpy(p.data, data, strlen(data) + HEADER_SIZE);

//     ssize_t numBytes = send(sock, &p, strlen(p.data) + HEADER_SIZE, 0);
//     if (numBytes < 0)
//         DieWithError("send() failed");
//     else if (numBytes != strlen(p.data) + HEADER_SIZE)
//         DieWithError("send() incorrect numbaer of bytes");

//     return numBytes;
// }

// struct Packet receivePacket(int sock) {
//     struct Packet* p;
//     char buffer[BUFFSIZE]; // Buffer for the received message
//     unsigned int numBytes = 0;
//     unsigned int totalBytes = 0;

//     // Receive message from client
//     for(;;) {
//         numBytes = recv(sock, buffer + numBytes, BUFFSIZE - 1, 0);
//         if (numBytes < 0)
//             DieWithError("recv() failed");
//         else if (numBytes == 0)
//             DieWithError("recv() connection closed prematurely");
//         totalBytes += numBytes;

//         // Stop when newline char is received
//         if(buffer[totalBytes - 1] == '\n') {
//             buffer[totalBytes - 1] = '\0';
//             break;
//         }
//     }

//     p = (struct Packet*) buffer;
//     return *p;
// }


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

    // Setup connection
    int sock = setupConnection(serverHost, servPortString);

    // START OF LOGON CODE ---------------------------------------------------

    // struct Packet logon;
    // logon.type = 0x00;
    // logon.length = 0x01;
    // char temp[SHORT_BUFFSIZE];
    // snprintf(temp, strlen(username) + strlen(password) + 2, "%s:%s", username, password);
    // strncat(logon.data, temp, strlen(temp));
    // strncat(logon.data, "\n", 1);

    // printf("%s\n", logon.data);

    // send(sock, &logon, strlen(logon.data) + HEADER_SIZE, 0);

    // END OF LOGON CODE ------------------------------------------------------


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

    for (;;){

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
        }

        // DIFF REQUEST
        else if (!strcmp(command, "DIFF")){
            printf("diff correct\n");
        }

        // PULL (SYNC) REQUEST
        else if (!strcmp(command, "PULL")){
            printf("pull correct\n");
        }

        // LEAVE REQUEST
        else if (!strcmp(command, "LEAVE")){
            printf("leave correct\n");
            close(sock);
            exit(0);
        }

        // INVALID COMMAND
        else{
            printf("Invalid input, please input command again\n");
            printf("Valid commands are: LIST, DIFF, PULL, and LEAVE\n");
        }
    }






    // START OF LOGON CODE ---------------------------------------------------

    // size_t num = 2;
    // struct Packet logon;
    // logon.type = 0b00000000;
    // logon.length = (u_char) num;
    // char temp[SHORT_BUFFSIZE];
    // snprintf(temp, strlen(username)+strlen(password), "%s:%s", username, password);
    // strncat(logon.data, temp, strlen(temp));
    // strncat(logon.data, "\n", 1);

    // send(sock, &logon, strlen(logon.data) + 2, 0);

    // END OF LOGON CODE ------------------------------------------------------

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