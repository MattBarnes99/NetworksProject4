#include "NetworkHeader.h"

// Utility funcitons
size_t sendPacket(int sock, u_char type, u_char length, char *data);
struct Packet receivePacket(int sock);
int setupConnection(char *serverHost, char *serverPortString);