#ifndef STOPWAIT_ARQ_COMMON_H
#define STOPWAIT_ARQ_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9003
#define MAX_DATA 100

#define DATA_PACKET 1
#define END_PACKET  2

typedef struct
{
    int type;
    int seqNo;
    char data[MAX_DATA];
} Packet;

typedef struct
{
    int ackNo;
} AckPacket;

#endif

