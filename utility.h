#include "NetworkHeader.h"
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <dirent.h>


// Utility funcitons
size_t sendPacket(int sock, u_char type, u_char length, char *data);
struct Packet receivePacket(int sock);
int setupConnection(char *serverHost, char *serverPortString);
unsigned int receiveFile(char *path, int size, int sock);
void calculateFileHash(char *path, char *hash);
char **listDir(char *path, int numFiles);
int countFilesInDir(char *path);