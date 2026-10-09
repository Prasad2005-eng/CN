#ifndef FHSS_COMMON_H
#define FHSS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9999
#define MAX_PACKETS 100
#define END -1

// Available frequency bands
int freqBands[8] = {
    200, 300, 400, 500,
    600, 700, 800, 900
};

// Frequency hopping sequence
int hopSequence[8] = {
    0, 4, 2, 7, 1, 5, 3, 6
};

// Packet structure
typedef struct
{
    int seqNo;
    int frequency;
    char data[4];       // 3 bits + '\0'
} Packet;

#endif
