#include "flsm_common.h"
#include <stdint.h>

int main()
{
    int sockfd;
    struct sockaddr_in serverAddr;
    struct sockaddr_in senderAddr;

    socklen_t addrLen = sizeof(senderAddr);

    FLSMRequest request;
    FLSMResult results[MAX_SUBNETS];

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(
            sockfd,
            (struct sockaddr *)&serverAddr,
            sizeof(serverAddr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    printf("FLSM Receiver started...\n");
    printf("Waiting for sender...\n");

    // Receive request
    recvfrom(
        sockfd,
        &request,
        sizeof(request),
        0,
        (struct sockaddr *)&senderAddr,
        &addrLen
    );

    printf("\nReceived Network Address : %s\n", request.network);
    printf("Received Number of Subnets : %d\n",
           request.subnetCount);

    /*
        Original network:
        192.168.0.0/24

        We need 8 subnets.

        2^3 = 8

        Therefore borrow 3 bits.

        New prefix:
        24 + 3 = /27
    */

    int originalPrefix = 24;

    int borrowedBits = 0;
    int numberOfSubnets = 1;

    while (numberOfSubnets < request.subnetCount)
    {
        numberOfSubnets = numberOfSubnets * 2;
        borrowedBits++;
    }

    // Check if exact power of 2
    if (numberOfSubnets != request.subnetCount)
    {
        printf("Number of subnets must be a power of 2.\n");

        int resultCount = 0;

        sendto(
            sockfd,
            &resultCount,
            sizeof(resultCount),
            0,
            (struct sockaddr *)&senderAddr,
            addrLen
        );

        close(sockfd);
        return 1;
    }

    int newPrefix = originalPrefix + borrowedBits;

    int hostBits = 32 - newPrefix;

    int addressesPerSubnet = 1 << hostBits;

    /*
        Create subnet mask.

        Example:

        /27
        11111111.11111111.11111111.11100000

        = 255.255.255.224
    */

    uint32_t mask;

    if (newPrefix == 32)
    {
        mask = 0xFFFFFFFF;
    }
    else
    {
        mask = 0xFFFFFFFF << (32 - newPrefix);
    }

    // Convert mask to IP format
    struct in_addr maskAddr;

    maskAddr.s_addr = htonl(mask);

    char subnetMask[20];

    inet_ntop(
        AF_INET,
        &maskAddr,
        subnetMask,
        sizeof(subnetMask)
    );

    // Convert network address to integer
    struct in_addr networkAddr;

    if (inet_pton(
            AF_INET,
            request.network,
            &networkAddr) <= 0)
    {
        printf("Invalid IP address.\n");

        close(sockfd);
        return 1;
    }

    uint32_t networkIP = ntohl(networkAddr.s_addr);

    // Find actual network address
    networkIP = networkIP & mask;

    /*
        Example:

        First subnet starts at:

        192.168.0.0

        Each subnet has:

        32 addresses

        Therefore:

        Subnet 1 -> 0
        Subnet 2 -> 32
        Subnet 3 -> 64
        ...
    */

    for (int i = 0; i < request.subnetCount; i++)
    {
        uint32_t subnetNetwork =
            networkIP + (i * addressesPerSubnet);

        uint32_t firstIP = subnetNetwork + 1;

        uint32_t lastIP =
            subnetNetwork + addressesPerSubnet - 2;

        // Convert first IP
        struct in_addr firstAddr;

        firstAddr.s_addr = htonl(firstIP);

        inet_ntop(
            AF_INET,
            &firstAddr,
            results[i].firstIP,
            sizeof(results[i].firstIP)
        );

        // Convert last IP
        struct in_addr lastAddr;

        lastAddr.s_addr = htonl(lastIP);

        inet_ntop(
            AF_INET,
            &lastAddr,
            results[i].lastIP,
            sizeof(results[i].lastIP)
        );

        // Store subnet number
        results[i].subnetNo = i + 1;

        // Store prefix
        snprintf(
    results[i].prefix,
    sizeof(results[i].prefix),
    "/%d",
    newPrefix
);
       

        // Store subnet mask
        strcpy(
            results[i].subnetMask,
            subnetMask
        );
    }

    // Print receiver-side calculation
    printf("\n===============================================\n");
    printf("        FLSM CALCULATION\n");
    printf("===============================================\n");

    printf("Original Prefix       : /24\n");
    printf("Number of Subnets     : %d\n",
           request.subnetCount);
    printf("Borrowed Bits         : %d\n",
           borrowedBits);
    printf("New Prefix            : /%d\n",
           newPrefix);
    printf("Addresses/Subnet      : %d\n",
           addressesPerSubnet);
    printf("Usable Hosts/Subnet   : %d\n",
           addressesPerSubnet - 2);
    printf("Subnet Mask           : %s\n",
           subnetMask);

    // Send number of results
    int resultCount = request.subnetCount;

    sendto(
        sockfd,
        &resultCount,
        sizeof(resultCount),
        0,
        (struct sockaddr *)&senderAddr,
        addrLen
    );

    // Send results
    sendto(
        sockfd,
        results,
        sizeof(results),
        0,
        (struct sockaddr *)&senderAddr,
        addrLen
    );

    printf("\nFLSM calculation completed.\n");

    close(sockfd);

    return 0;
}
