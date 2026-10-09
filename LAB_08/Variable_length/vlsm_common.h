#ifndef VLSM_COMMON_H
#define VLSM_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define VLSM_PORT 9011
#define MAX_SUBNETS 10

typedef struct
{
    char network[20];
    int subnetCount;
    int hosts[MAX_SUBNETS];
} VLSMRequest;

typedef struct
{
    int subnetNo;
    int requiredHosts;
    char network[20];
    char firstIP[20];
    char lastIP[20];
    char broadcast[20];
    char prefix[16];
    char subnetMask[20];
} VLSMResult;

typedef struct
{
    int count;
    int status;
    VLSMResult results[MAX_SUBNETS];
} VLSMResponse;

#endif
