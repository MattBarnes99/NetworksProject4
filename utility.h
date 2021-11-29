#include "NetworkHeader.h"

// UTILITY FUNCTIONS

//For sending generic packets
size_t sendPacket(int sock, u_char type, u_char length, char *data);
// To recieve packets
struct Packet receivePacket(int sock);
// to setup connection on client and server
int setupConnection(char *serverHost, char *serverPortString);
// to receive files on client and server
unsigned int receiveFile(char *path, int size, int sock);
// to send push packet from client and server
void sendPushPacket(char **filePaths, int num, int sock);
// to push files from client and server
void pushFiles(char **filePaths, int num, int sock);
// diff crawl
char **listDir(char* path, int* numFiles);
// create hash for robust file comparison
void createHash(char *path, char *hash);
