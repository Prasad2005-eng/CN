#include "vlsm_common.h"

int main(void)
{
    int sockfd;
    struct sockaddr_in receiverAddr;
    socklen_t addrLen = sizeof(receiverAddr);

    VLSMRequest request;
    VLSMResponse response;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&receiverAddr, 0, sizeof(receiverAddr));
    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(VLSM_PORT);
    inet_pton(AF_INET, "127.0.0.1",
              &receiverAddr.sin_addr);

    strcpy(request.network, "195.1.1.0");
    request.subnetCount = MAX_SUBNETS;

    printf("VLSM: Network = %s/24\n", request.network);
    printf("Number of subnets = %d\n", request.subnetCount);

    for (int i = 0; i < request.subnetCount; i++)
    {
        printf("Required hosts for subnet %d: ", i + 1);

        if (scanf("%d", &request.hosts[i]) != 1 ||
            request.hosts[i] < 1)
        {
            printf("Invalid host requirement.\n");
            close(sockfd);
            return 1;
        }
    }

    if (sendto(sockfd, &request, sizeof(request), 0,
               (struct sockaddr *)&receiverAddr,
               addrLen) != sizeof(request))
    {
        perror("Sending request failed");
        close(sockfd);
        return 1;
    }

    if (recvfrom(sockfd, &response, sizeof(response), 0,
                 (struct sockaddr *)&receiverAddr,
                 &addrLen) != sizeof(response))
    {
        perror("Receiving response failed");
        close(sockfd);
        return 1;
    }

    if (!response.status)
    {
        printf("\nVLSM allocation failed. Check host requirements.\n");
        close(sockfd);
        return 1;
    }

    printf("\n========== VLSM RESULTS ==========\n");
    printf("%-7s %-7s %-18s %-18s %-18s %-5s %-18s\n",
           "Subnet", "Hosts", "First IP", "Last IP",
           "Broadcast", "CIDR", "Subnet Mask");

    for (int i = 0; i < response.count; i++)
    {
        VLSMResult *r = &response.results[i];

        printf("%-7d %-7d %-18s %-18s %-18s %-5s %-18s\n",
               r->subnetNo, r->requiredHosts,
               r->firstIP, r->lastIP, r->broadcast,
               r->prefix, r->subnetMask);
    }

    close(sockfd);
    return 0;
}
