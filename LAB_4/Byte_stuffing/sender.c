#include "bytestuff_common.h"

int main()
{
    int sockfd;

    char data[BUFFER_SIZE];
    char stuffed_data[BUFFER_SIZE * 2];
    char frame[BUFFER_SIZE * 2 + 2];

    struct sockaddr_in receiver_addr;

    /*
     * Step 1: Read original data
     */
    printf("Enter data: ");

    if (fgets(data, sizeof(data), stdin) == NULL)
    {
        printf("Error: Unable to read input.\n");
        return 1;
    }

    /*
     * Remove newline added by fgets()
     */
    data[strcspn(data, "\n")] = '\0';

    /*
     * Step 2: Validate input
     */
    if (!is_valid_data(data))
    {
        printf("Error: Data cannot be empty.\n");
        return 1;
    }

    /*
     * Step 3: Perform byte stuffing
     */
    byte_stuff(data, stuffed_data);

    /*
     * Step 4: Display original and stuffed data
     */
    printf("\nOriginal Data : %s\n", data);
    printf("Stuffed Data  : %s\n", stuffed_data);

    /*
     * Step 5: Create complete frame
     *
     * Format:
     *
     * FLAG + STUFFED DATA + FLAG
     */
    snprintf(
        frame,
        sizeof(frame),
        "%c%s%c",
        FLAG,
        stuffed_data,
        FLAG
    );

    printf("Frame         : %s\n", frame);

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
     * Step 7: Initialize receiver address
     */
    memset(&receiver_addr, 0, sizeof(receiver_addr));

    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(PORT);

    /*
     * Receiver is running on the same machine.
     */
    if (inet_pton(
            AF_INET,
            "127.0.0.1",
            &receiver_addr.sin_addr
        ) <= 0)
    {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    /*
    *  Step 8:Send frame using UDP.

    * Send only the actual frame characters.
    * Do not send the C string terminator '\0'.
    */
    if (sendto(
 	     sockfd,
	     frame,
 	     strlen(frame),
	     0,
	     (struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) < 0)
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
