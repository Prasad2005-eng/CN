#include "routing_common.h"

int main(void)
{
    GraphRequest g;
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int sockfd = receiveGraph(
        BELLMAN_PORT, &g, &clientAddr, &clientLen
    );

    if (sockfd < 0)
        return 1;

    int dist[MAX_VERTICES];

    for (int i = 0; i < g.n; i++)
        dist[i] = INF;

    dist[g.source] = 0;

    for (int iteration = 1; iteration < g.n; iteration++)
    {
        int changed = 0;

        for (int i = 0; i < g.edgeCount; i++)
        {
            int u = g.edges[i].u;
            int v = g.edges[i].v;
            int w = g.edges[i].weight;

            if (dist[u] != INF &&
                (long long)dist[u] + w < dist[v])
            {
                dist[v] = dist[u] + w;
                changed = 1;
            }
        }

        if (!changed)
            break;
    }

    printf("\n===== BELLMAN-FORD: DISTANCE VECTOR =====\n");
    printf("Source vertex: %d\n", g.source);
    printf("Destination\tShortest cost\n");

    for (int i = 0; i < g.n; i++)
    {
        if (dist[i] == INF)
            printf("%d\t\tUnreachable\n", i);
        else
            printf("%d\t\t%d\n", i, dist[i]);
    }

    close(sockfd);
    return 0;
}
