#include "routing_common.h"

int main(void)
{
    GraphRequest g;
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int sockfd = receiveGraph(
        CYCLE_PORT, &g, &clientAddr, &clientLen
    );

    if (sockfd < 0)
        return 1;

    int dist[MAX_VERTICES];

    /*
       Initialize every vertex to zero.
       This allows detection of negative cycles
       even in disconnected components.
    */
    for (int i = 0; i < g.n; i++)
        dist[i] = 0;

    int hasNegativeCycle = 0;

    for (int iteration = 0; iteration < g.n; iteration++)
    {
        int changed = 0;

        for (int i = 0; i < g.edgeCount; i++)
        {
            int u = g.edges[i].u;
            int v = g.edges[i].v;
            int w = g.edges[i].weight;

            long long candidate = (long long)dist[u] + w;

            if (candidate < dist[v])
            {
                dist[v] = (int)candidate;
                changed = 1;
            }
        }

        if (!changed)
            break;

        if (iteration == g.n - 1)
            hasNegativeCycle = 1;
    }

    printf("\n===== NEGATIVE WEIGHT CYCLE DETECTION =====\n");

    if (hasNegativeCycle)
        printf("Negative weight cycle EXISTS in the graph.\n");
    else
        printf("No negative weight cycle found.\n");

    close(sockfd);
    return 0;
}
