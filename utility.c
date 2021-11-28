#include "utility.h"

// Send packet
size_t sendPacket(int sock, u_char type, u_char length, char *data) {
    struct Packet p;
    p.type = type;
    p.length = length;
    strncpy(p.data, data, strlen(data) + HEADER_SIZE);

    ssize_t numBytes = send(sock, &p, strlen(p.data) + HEADER_SIZE, 0);
    if (numBytes < 0)
        DieWithError("send() failed");
    else if (numBytes != strlen(p.data) + HEADER_SIZE)
        DieWithError("send() incorrect numbaer of bytes");

    return numBytes;
}

// Receive packet
struct Packet receivePacket(int sock) {
    struct Packet* p;
    char buffer[BUFFSIZE]; // Buffer for the received message
    unsigned int numBytes = 0;
    unsigned int totalBytes = 0;

    // Receive message from client
    for(;;) {
        numBytes = recv(sock, buffer + numBytes, BUFFSIZE - 1, 0);
        if (numBytes < 0)
            DieWithError("recv() failed");
        else if (numBytes == 0)
            DieWithError("recv() connection closed prematurely");
        totalBytes += numBytes;

        // Stop when newline char is received
        if(buffer[totalBytes - 1] == '\n') {
            buffer[totalBytes - 1] = '\0';
            break;
        }
    }

    p = (struct Packet*) buffer;
    return *p;
}

// setup TCP connection
int setupConnection(char *serverHost, char *serverPortString) {
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
    int rtnVal = getaddrinfo(serverHost, serverPortString, &addrCriteria, &addrList);
    if (rtnVal != 0)
        DieWithError("getaddrinfo() failed");

    struct addrinfo *addr = addrList;

    // Establish the connection to the server
    if (connect(sock, addr->ai_addr, addr->ai_addrlen) < 0)
        DieWithError("connect() failed");

    return sock;
}