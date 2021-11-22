#include "NetworkHeader.h"


void DieWithError(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}


int main(int argc, char *argv[]) {

    char *serverHost = SERVER_HOST;
    char *servPortString = SERVER_PORT;
    // char *fileName;

    // Test for correct number of arguments
    if (argc != 5)
        DieWithError("Usage Project4Server -s <cookie> -p <port>");

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



    size_t num = 3;
    char fileNames[3][100] = {"sample1.mp3", "sample2.mp3", "sample3.mp3"};
    int fileSizes[(int) num];
    struct Packet sending;
    sending.type = 0b00000010;
    sending.length = (u_char) num;

    for (size_t i = 0; i < num; i++) {
        int fd = open(fileNames[i], O_RDONLY);
        struct stat file_stat;
        fstat(fd, &file_stat);
        fileSizes[i] = file_stat.st_size;
        char temp[SHORT_BUFFSIZE];
        snprintf(temp, sizeof(fileNames[i])+32, "%s:%d:", fileNames[i], fileSizes[i]);
        strncat(sending.data, temp, strlen(temp));
    }
    strncat(sending.data, "\n", 1);
    printf("%s\n", sending.data);
    printf("%d", send(sock, &sending, strlen(sending.data) + 2, 0));

    for (size_t i = 0; i < num; i++) {

        int fd = open(fileNames[i], O_RDONLY);
        struct stat file_stat;
        fstat(fd, &file_stat);
        printf("File: %s ; File size: %zu\n", fileNames[i], file_stat.st_size);

        off_t offset = 0;
        int sent_bytes = 0;
        int remain_data = file_stat.st_size;

        /* Sending file data */
        while (((sent_bytes = sendfile(sock, fd, &offset, BUFFSIZE)) > 0) && (remain_data > 0)) {
                printf("Sent: %d ; Offset: %ld ; Remaining: %d\n", sent_bytes, offset, remain_data);
                remain_data -= sent_bytes;
        }
        sleep(2);
    }

    for(;;) {
        sleep(1);
    }
    return 0;
}


// void recievePush(char buffer[]){

// }