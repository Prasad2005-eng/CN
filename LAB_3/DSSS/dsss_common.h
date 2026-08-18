#ifndef DSSS_COMMON_H
#define DSSS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8888
#define MAX_PACKETS 100
#define END -1

// PN sequence
char pnCode[] = "1011";

// Packet structure
typedef struct
{
    int seqNo;
    char data[5];       // 4 spread bits + '\0'
} Packet;

#endif
