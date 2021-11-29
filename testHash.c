#include <stdio.h>
#include <openssl/md5.h>
#include "NetworkHeader.h"

void createHash(char *path, char *hash) {
    unsigned char c[MD5_DIGEST_LENGTH];
    int i;
    FILE *mp3file = fopen(path, "rb");
    MD5_CTX md;
    int bytes = 0;
    unsigned char data[BUFFERSIZE];

    MD5_Init(&md);
    while ((bytes = fread(data, 1, BUFFERSIZE, mp3file)) != 0)
        MD5_Update(&md, data, bytes);
    MD5_Final(c, &md);

    for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(&hash[i * 2], "%02x", c[i]);
    }

    fclose(mp3file);
}

int main() {
    char *p = "Matt_Client/sample3.mp3";
    char h[128];
    memset(h, 0, sizeof(h));
    createHash(p, h);
    printf("Saved: %s\n", h);
}

/*
//  FILE *mp3file;
//     unsigned char md5_digest[MD5_DIGEST_LENGTH];

//     mp3file = fopen(path, "rb");

//     uint8_t buffer[BUFFERSIZE];

//     for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
//     {
//         printf("%02x", (unsigned int)md5_digest[i]);
//         //char *p = (char *) &md5_digest[i];
//         // printf("%s\n",p);
//         sprintf(&hash[i * 2], "%02x", md5_digest[i]);
//         //strncpy(hash, p, strlen(p));
//     }
//     printf("\n");
//     printf("Saved: %s\n", hash);

//     int bytes;
//     MD5_CTX mdContext;
//     MD5_Init(&mdContext);
//     while ((bytes = fread(buffer, 1, 1024, mp3file)) != 0)
//         MD5_Update(&mdContext, buffer, bytes);
//     MD5_Final(md5_digest, &mdContext);
//     for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
//         printf("%02x", md5_digest[i]);
//         //sprintf(&hash[i * 2], "%02x", md5_digest[i]);
//     printf(" %s\n", path);
//     fclose(mp3file);
//     printf("\n");
//     printf("Saved: %s\n", hash);*/