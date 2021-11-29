#include "NetworkHeader.h"

// Utility funcitons
size_t sendPacket(int sock, u_char type, u_char length, char *data);
struct Packet receivePacket(int sock);
int setupConnection(char *serverHost, char *serverPortString);
unsigned int receiveFile(char *path, int size, int sock);
void sendPushPacket(char **filePaths, int num, int sock);
void pushFiles(char **filePaths, int num, int sock);
char **listDir(char* path, int* numFiles);
void createHash(char *path, char *hash);
