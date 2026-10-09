#ifndef ROUTING_COMMON_H
#define ROUTING_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define DIJKSTRA_PORT 9012
#define BELLMAN_PORT 9013
#define CYCLE_PORT 9014

#define MAX_VERTICES 10
#define MAX_EDGES 100
#define INF 1000000000

typedef struct
{
    int u;
    int v;
    int weight;
} Edge;

typedef struct
{
    int n;
    int edgeCount;
    int source;
    Edge edges[MAX_EDGES];
} GraphRequest;

static int receiveGraph(int port, GraphRequest *g,
                        struct sockaddr_in *clientAddr,
                        socklen_t *clientLen)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        return -1;
    }

    printf("Receiver waiting on UDP port %d...\n", port);

    ssize_t received = recvfrom(
        sockfd, g, sizeof(*g), 0,
        (struct sockaddr *)clientAddr, clientLen
    );

    if (received != sizeof(*g))
    {
        fprintf(stderr, "Invalid graph request received.\n");
        close(sockfd);
        return -1;
    }

    if (g->n < 1 || g->n > MAX_VERTICES ||
        g->edgeCount < 0 || g->edgeCount > MAX_EDGES ||
        g->source < 0 || g->source >= g->n)
    {
        fprintf(stderr, "Invalid graph parameters.\n");
        close(sockfd);
        return -1;
    }

    for (int i = 0; i < g->edgeCount; i++)
    {
        if (g->edges[i].u < 0 || g->edges[i].u >= g->n ||
            g->edges[i].v < 0 || g->edges[i].v >= g->n ||
            g->edges[i].weight <= -INF ||
            g->edges[i].weight >= INF)
        {
            fprintf(stderr, "Invalid edge data.\n");
            close(sockfd);
            return -1;
        }
    }

    return sockfd;
}

#endif
