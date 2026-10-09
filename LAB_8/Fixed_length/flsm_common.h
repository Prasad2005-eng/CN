#ifndef FLSM_COMMON_H
#define FLSM_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9010
#define MAX_SUBNETS 16

typedef struct
{
    char network[20];
    int subnetCount;
} FLSMRequest;

typedef struct
{
    int subnetNo;
    char firstIP[20];
    char lastIP[20];
    char prefix[10];
    char subnetMask[20];
} FLSMResult;

#endif
