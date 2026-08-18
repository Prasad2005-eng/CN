#include "bytestuff_common.h"

int main()
{
    int sockfd;

    char frame[BUFFER_SIZE * 2 + 3];
    char stuffed_data[BUFFER_SIZE * 2 + 1];
    char original_data[BUFFER_SIZE * 2 + 1];

    struct sockaddr_in server_addr;
    struct sockaddr_in sender_addr;

    socklen_t sender_len = sizeof(sender_addr);

    /*
     * Step 1: Create UDP socket
     */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    /*
     * Step 2: Clear server address structure
     */
    memset(&server_addr, 0, sizeof(server_addr));

    /*
     * Step 3: Configure server address
     */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    /*
     * Step 4: Bind socket to port
     */
    if (bind(
            sockfd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)
        ) < 0)
    {
        perror("bind");
        close(sockfd);
        return 1;
    }

    printf("Byte Stuffing Receiver\n");
    printf("----------------------\n");
    printf("Listening on UDP port %d...\n\n", PORT);

    /*
     * Step 5: Receive frame
     */
    int bytes_received = recvfrom(
        sockfd,
        frame,
        sizeof(frame) - 1,
        0,
        (struct sockaddr *)&sender_addr,
        &sender_len
    );

    if (bytes_received < 0)
    {
        perror("recvfrom");
        close(sockfd);
        return 1;
    }

    /*
     * Make received data a valid C string.
     */
    frame[bytes_received] = '\0';

    printf("Received Frame : %s\n", frame);

    /*
     * Step 6: Check minimum frame size
     *
     * Minimum valid frame:
     *
     * FLAG + FLAG
     *
     * Since FLAG is one character,
     * minimum length is 2.
     */
    if (bytes_received < 2)
    {
        printf("Error: Frame is too short.\n");
        close(sockfd);
        return 1;
    }

    /*
     * Step 7: Check starting FLAG
     */
    if (frame[0] != FLAG)
    {
        printf("Error: Invalid starting FLAG.\n");
        close(sockfd);
        return 1;
    }

    printf("Starting FLAG detected.\n");

    /*
     * Step 8: Check ending FLAG
     */
    if (frame[bytes_received - 1] != FLAG)
    {
        printf("Error: Invalid ending FLAG.\n");
        close(sockfd);
        return 1;
    }

    printf("Ending FLAG detected.\n");

    /*
     * Step 9: Remove starting and ending FLAG
     *
     * Example:
     *
     * F + ABEFCEE + F
     *
     * becomes:
     *
     * ABEFCEE
     */
    int stuffed_length = bytes_received - 2;

    if (stuffed_length == 0)
    {
        printf("Error: Frame contains no data.\n");
        close(sockfd);
        return 1;
    }

    memcpy(
        stuffed_data,
        frame + 1,
        stuffed_length
    );

    stuffed_data[stuffed_length] = '\0';

    printf("\nStuffed Data   : %s\n", stuffed_data);

    /*
     * Step 10: Perform byte destuffing
     */
    if (!byte_destuff(stuffed_data, original_data))
    {
        printf("Error: Malformed stuffed data.\n");
        close(sockfd);
        return 1;
    }

    /*
     * Step 11: Display recovered data
     */
    printf("Recovered Data : %s\n", original_data);

    printf("\nByte destuffing successful.\n");
    printf("Original data recovered successfully.\n");

    /*
     * Step 12: Close socket
     */
    close(sockfd);

    return 0;
}
