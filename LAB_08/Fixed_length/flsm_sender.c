#include "flsm_common.h"

int main()
{
    int sockfd;
    struct sockaddr_in receiverAddr;

    FLSMRequest request;
    FLSMResult results[MAX_SUBNETS];

    socklen_t addrLen = sizeof(receiverAddr);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Receiver address
    memset(&receiverAddr, 0, sizeof(receiverAddr));

    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(PORT);
    receiverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Input
    printf("Enter network address: ");
    scanf("%19s", request.network);

    printf("Enter number of subnets: ");
    scanf("%d", &request.subnetCount);

    // Send request to receiver
    sendto(
        sockfd,
        &request,
        sizeof(request),
        0,
        (struct sockaddr *)&receiverAddr,
        addrLen
    );

    // Receive results from receiver
    int resultCount;

    recvfrom(
        sockfd,
        &resultCount,
        sizeof(resultCount),
        0,
        (struct sockaddr *)&receiverAddr,
        &addrLen
    );

    recvfrom(
        sockfd,
        results,
        sizeof(results),
        0,
        (struct sockaddr *)&receiverAddr,
        &addrLen
    );

    // Display result
    printf("\n===============================================\n");
    printf("        FIXED LENGTH SUBNETTING (FLSM)\n");
    printf("===============================================\n");

    printf("\nNetwork Address : %s\n", request.network);
    printf("Number of Subnets : %d\n", request.subnetCount);

    printf("\n%-8s %-18s %-18s %-8s %-20s\n",
           "Subnet",
           "First IP",
           "Last IP",
           "Prefix",
           "Subnet Mask");

    printf("--------------------------------------------------------------------------\n");

    for (int i = 0; i < resultCount; i++)
    {
        printf("%-8d %-18s %-18s %-8s %-20s\n",
               results[i].subnetNo,
               results[i].firstIP,
               results[i].lastIP,
               results[i].prefix,
               results[i].subnetMask);
    }

    printf("--------------------------------------------------------------------------\n");

    close(sockfd);

    return 0;
}
