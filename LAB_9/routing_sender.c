#include "routing_common.h"

int main(void)
{
    GraphRequest g;
    int choice, port;

    printf("ROUTING ALGORITHMS\n");
    printf("1. Dijkstra - Link State\n");
    printf("2. Bellman-Ford - Distance Vector\n");
    printf("3. Negative Weight Cycle Detection\n");
    printf("Choose algorithm: ");

    if (scanf("%d", &choice) != 1 ||
        choice < 1 || choice > 3)
    {
        printf("Invalid choice.\n");
        return 1;
    }

    if (choice == 1)
        port = DIJKSTRA_PORT;
    else if (choice == 2)
        port = BELLMAN_PORT;
    else
        port = CYCLE_PORT;

    memset(&g, 0, sizeof(g));

    printf("Enter number of vertices (maximum %d): ",
           MAX_VERTICES);
    if (scanf("%d", &g.n) != 1 ||
        g.n < 1 || g.n > MAX_VERTICES)
    {
        printf("Invalid vertex count.\n");
        return 1;
    }

    printf("Enter number of directed edges (maximum %d): ",
           MAX_EDGES);
    if (scanf("%d", &g.edgeCount) != 1 ||
        g.edgeCount < 0 || g.edgeCount > MAX_EDGES)
    {
        printf("Invalid edge count.\n");
        return 1;
    }

    printf("Enter source vertex (0 to %d): ", g.n - 1);
    if (scanf("%d", &g.source) != 1 ||
        g.source < 0 || g.source >= g.n)
    {
        printf("Invalid source vertex.\n");
        return 1;
    }

    printf("\nEnter each edge as: from to weight\n");
    printf("Vertices are numbered 0 to %d.\n", g.n - 1);

    for (int i = 0; i < g.edgeCount; i++)
    {
        printf("Edge %d: ", i + 1);

        if (scanf("%d %d %d",
                  &g.edges[i].u,
                  &g.edges[i].v,
                  &g.edges[i].weight) != 3)
        {
            printf("Invalid edge input.\n");
            return 1;
        }

        if (g.edges[i].u < 0 || g.edges[i].u >= g.n ||
            g.edges[i].v < 0 || g.edges[i].v >= g.n ||
            g.edges[i].weight <= -INF ||
            g.edges[i].weight >= INF)
        {
            printf("Invalid edge values.\n");
            return 1;
        }
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in receiverAddr;
    memset(&receiverAddr, 0, sizeof(receiverAddr));

    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1",
              &receiverAddr.sin_addr);

    if (sendto(sockfd, &g, sizeof(g), 0,
               (struct sockaddr *)&receiverAddr,
               sizeof(receiverAddr)) != sizeof(g))
    {
        perror("Sending graph failed");
        close(sockfd);
        return 1;
    }

    printf("\nGraph sent successfully to receiver on port %d.\n",
           port);

    close(sockfd);
    return 0;
}
