#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>

#define PORT 9020
#define CHUNK_SIZE 1024
#define MAX_FILENAME 256
#define MAX_RETRIES 5

#define START_PACKET 1
#define DATA_PACKET  2
#define END_PACKET   3

typedef struct
{
    int type;
    int seq;
    char filename[MAX_FILENAME];
    uint64_t fileSize;
    int dataLen;
    char data[CHUNK_SIZE];
} FilePacket;

typedef struct
{
    int ack;
} AckPacket;

#endif
