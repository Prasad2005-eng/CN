#include "vlsm_common.h"

typedef struct
{
    int subnetNo;
    int hosts;
} SubnetRequest;

static int compareHosts(const void *a, const void *b)
{
    const SubnetRequest *x = a;
    const SubnetRequest *y = b;

    if (x->hosts < y->hosts) return 1;
    if (x->hosts > y->hosts) return -1;
    return 0;
}

static void ipString(uint32_t ip, char *out, size_t size)
{
    struct in_addr addr;
    addr.s_addr = htonl(ip);
    inet_ntop(AF_INET, &addr, out, size);
}

int main(void)
{
    int sockfd;
    struct sockaddr_in serverAddr, senderAddr;
    socklen_t addrLen = sizeof(senderAddr);

    VLSMRequest request;
    VLSMResponse response;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(VLSM_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    printf("VLSM receiver waiting on port %d...\n", VLSM_PORT);

    if (recvfrom(sockfd, &request, sizeof(request), 0,
                 (struct sockaddr *)&senderAddr,
                 &addrLen) != sizeof(request))
    {
        perror("Receiving request failed");
        close(sockfd);
        return 1;
    }

    memset(&response, 0, sizeof(response));
    response.status = 0;

    if (request.subnetCount != MAX_SUBNETS)
    {
        goto send_response;
    }

    struct in_addr addr;

    if (inet_pton(AF_INET, request.network, &addr) != 1)
    {
        goto send_response;
    }

    /*
       This practical uses network 195.1.1.0/24.
       VLSM allocations must fit inside its 256 addresses.
    */
    uint32_t base = ntohl(addr.s_addr);
    uint32_t end = base + 256U;

    if ((base & 255U) != 0 || end > 0x100000000ULL)
    {
        goto send_response;
    }

    SubnetRequest subs[MAX_SUBNETS];

    for (int i = 0; i < MAX_SUBNETS; i++)
    {
        if (request.hosts[i] < 1 || request.hosts[i] > 254)
        {
            goto send_response;
        }

        subs[i].subnetNo = i + 1;
        subs[i].hosts = request.hosts[i];
    }

    qsort(subs, MAX_SUBNETS, sizeof(subs[0]), compareHosts);

    uint32_t current = base;

    for (int i = 0; i < MAX_SUBNETS; i++)
    {
        int hostBits = 2;

        while (((1ULL << hostBits) - 2) <
               (unsigned long long)subs[i].hosts)
        {
            hostBits++;
        }

        int prefix = 32 - hostBits;
        uint32_t blockSize = 1U << hostBits;

        if ((uint64_t)current + blockSize > end)
        {
            goto send_response;
        }

        VLSMResult *r = &response.results[i];

        r->subnetNo = subs[i].subnetNo;
        r->requiredHosts = subs[i].hosts;

        ipString(current, r->network, sizeof(r->network));
        ipString(current + 1U, r->firstIP, sizeof(r->firstIP));
        ipString(current + blockSize - 2U,
                 r->lastIP, sizeof(r->lastIP));
        ipString(current + blockSize - 1U,
                 r->broadcast, sizeof(r->broadcast));

        snprintf(r->prefix, sizeof(r->prefix), "/%d", prefix);

        uint32_t mask = 0xFFFFFFFFU << hostBits;
        ipString(mask, r->subnetMask, sizeof(r->subnetMask));

        printf("Allocated subnet %d: %s/%d, hosts required: %d\n",
               r->subnetNo, r->network, prefix, r->requiredHosts);

        current += blockSize;
    }

    response.count = MAX_SUBNETS;
    response.status = 1;

send_response:
    if (sendto(sockfd, &response, sizeof(response), 0,
               (struct sockaddr *)&senderAddr,
               addrLen) != sizeof(response))
    {
        perror("Sending response failed");
        close(sockfd);
        return 1;
    }

    if (!response.status)
    {
        printf("VLSM failed: invalid input or insufficient address space.\n");
    }
    else
    {
        printf("VLSM allocation completed successfully.\n");
    }

    close(sockfd);
    return 0;
}
