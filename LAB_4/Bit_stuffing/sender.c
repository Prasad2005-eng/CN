#include "bitstuff_common.h"

int main()
{
    int sockfd;

    char data[BUFFER_SIZE];
    char stuffed_data[BUFFER_SIZE * 2];
    char frame[BUFFER_SIZE * 2 + 20];

    struct sockaddr_in receiver_addr;

    /*
     * Step 1: Read binary data from user
     */
    printf("Enter binary data (0s and 1s): ");
    scanf("%1023s", data);

    /*
     * Step 2: Validate input
     */
    if (!is_binary(data))
    {
        printf("Error: Input must contain only 0 and 1.\n");
        return 1;
    }

    /*
     * Step 3: Perform bit stuffing
     */
    bit_stuff(data, stuffed_data);

    /*
     * Step 4: Display original and stuffed data
     */
    printf("\nOriginal Data : %s\n", data);
    printf("Stuffed Data  : %s\n", stuffed_data);

    /*
     * Step 5: Create complete frame
     *
     * Frame format:
     *
     * FLAG + STUFFED DATA + FLAG
     */
    snprintf(frame, sizeof(frame), "%s%s%s",
             FLAG, stuffed_data, FLAG);

    printf("Transmitted Frame: %s\n", frame);

    /*
     * Step 6: Create UDP socket
     */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    /*
     * Step 7: Configure receiver address
     */
    memset(&receiver_addr, 0, sizeof(receiver_addr));

    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(PORT);

    /*
     * 127.0.0.1 means this computer itself.
     */
    if (inet_pton(AF_INET, "127.0.0.1",
                  &receiver_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    /*
     * Step 8: Send frame to receiver
     */
    if (sendto(sockfd,
               frame,
               strlen(frame) + 1,
               0,
               (struct sockaddr *)&receiver_addr,
               sizeof(receiver_addr)) < 0)
    {
        perror("sendto");
        close(sockfd);
        return 1;
    }

    printf("\nFrame sent successfully.\n");

    /*
     * Step 9: Close socket
     */
    close(sockfd);

    return 0;
}
