#include "routing_common.h"

int main(void)
{
    GraphRequest g;
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int sockfd = receiveGraph(
        DIJKSTRA_PORT, &g, &clientAddr, &clientLen
    );

    if (sockfd < 0)
        return 1;

    int dist[MAX_VERTICES];
    int visited[MAX_VERTICES] = {0};

    for (int i = 0; i < g.n; i++)
        dist[i] = INF;

    dist[g.source] = 0;

    int valid = 1;

    for (int i = 0; i < g.edgeCount; i++)
    {
        if (g.edges[i].weight < 0)
        {
            valid = 0;
            break;
        }
    }

    if (!valid)
    {
        printf("Dijkstra cannot handle negative edge weights.\n");
        close(sockfd);
        return 1;
    }

    for (int count = 0; count < g.n; count++)
    {
        int u = -1;

        for (int v = 0; v < g.n; v++)
        {
            if (!visited[v] &&
                (u == -1 || dist[v] < dist[u]))
                u = v;
        }

        if (u == -1 || dist[u] == INF)
            break;

        visited[u] = 1;

        for (int i = 0; i < g.edgeCount; i++)
        {
            int from = g.edges[i].u;
            int to = g.edges[i].v;
            int weight = g.edges[i].weight;

            if (from == u && !visited[to] &&
                dist[u] <= INF - weight &&
                dist[u] + weight < dist[to])
            {
                dist[to] = dist[u] + weight;
            }
        }
    }

    printf("\n===== DIJKSTRA: LINK STATE ROUTING =====\n");
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
