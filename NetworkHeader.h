#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <signal.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <ctype.h>
#include <stdbool.h>
#include <dirent.h>

#define SERVER_HOST "141.166.206.223"  /* wallis IP address */
#define SERVER_PORT "35001"

#define SA struct sockaddr

/* Miscellaneous constants */
#define	MAXLINE		4096	/* max text line length */
#define	MAXSOCKADDR  128	/* max socket address structure size */
#define	BUFFSIZE	8192	/* buffer size for reads and writes */
#define	LISTENQ		1024	/* 2nd argument to listen() */
#define SHORT_BUFFSIZE  100     /* For messages I know are short */
void DieWithError(char *errorMessage); /*Error handling function */

// Packet properties
#define HEADER_SIZE     0x02
#define DEFAULT_LENGTH  0x00
#define DEFAULT_MESSAGE "\n"

// Packet types
#define LOGON_TYPE   0x00
#define LIST_TYPE    0x01
#define PUSH_TYPE    0x02
#define PULL_TYPE    0x03
#define LEAVE_TYPE   0x04

#define ACK_TYPE     0x05
#define NACK_TYPE    0x06

struct Packet {
    u_char type;
    u_char length;
    char data[BUFFSIZE];
};
