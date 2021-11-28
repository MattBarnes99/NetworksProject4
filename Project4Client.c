#include "NetworkHeader.h"


void DieWithError(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}


int main(int argc, char *argv[]) {

    char *serverHost = SERVER_HOST;
    char *servPortString = SERVER_PORT;
    char *username;
    char *password;
    char *command;
    // char *fileName;

    // Test for correct number of arguments
    if (argc != 9)
        DieWithError("Usage Project4Server -h <IP/hostname> -p <port> -u <Username> -t <password>");

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
                case 'u':
                    username = argv[i + 1];
                    break;
                case 't':
                    password = argv[i + 1];
                    break;
                // case 'f':
                //     fileName = argv[i + 1];
                //     break;
                default:
                    break;
            }
        }
    }

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
    int rtnVal = getaddrinfo(serverHost, servPortString, &addrCriteria, &addrList);
    if (rtnVal != 0)
        DieWithError("getaddrinfo() failed");

    struct addrinfo *addr = addrList;

    // Establish the connection to the server
    if (connect(sock, addr->ai_addr, addr->ai_addrlen) < 0)
        DieWithError("connect() failed");

        

    for (;;){

        printf("Please enter a command (in captial letters):\n");
        scanf("%s",command);
        
        // LIST REQUET
        if (strcmp(command,"LIST") == 0){
            printf("list correct");
        }

        // DIFF REQUEST
        else if (strcmp(command,"DIFF") == 0){
            printf("diff correct");
        }

        // PULL (SYNC) REQUEST
        else if (strcmp(command,"PULL") == 0){
            printf("pull correct");
        }

        // LEAVE REQUEST
        else if (strcmp(command,"LEAVE") == 0){
            printf("leave correct");
        }

        // INVALID COMMAND
        else{
            printf("Invalid input, please input command again\n");
            printf("Valid commands are: LIST, DIFF, PULL, and LEAVE");
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

