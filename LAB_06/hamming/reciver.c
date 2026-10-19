#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;

    int total;
    char hamming[100];

    socklen_t addr_size = sizeof(client_addr);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    // Listen
    listen(server_fd, 5);

    printf("Server waiting for connection...\n");

    // Accept
    client_fd = accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &addr_size);

    if (client_fd < 0)
    {
        perror("Accept failed");
        close(server_fd);
        return 1;
    }

    // Receive total number of bits
    recv(client_fd, &total, sizeof(total), 0);

    // Receive Hamming code
    recv(client_fd, hamming + 1, total, 0);

    hamming[total + 1] = '\0';

    printf("\n----- HAMMING CODE RECEIVER -----\n");

    printf("Received Hamming Code: ");

    for (int pos = total; pos >= 1; pos--)
        printf("%c", hamming[pos]);

    printf("\n");

    /*
        Calculate number of parity bits
    */

    int r = 0;

    while ((1 << r) < total + 1)
    {
        r++;
    }

    int error_position = 0;

    /*
        Check P1, P2, P4, P8...
    */

    for (int p = 0; p < r; p++)
    {
        int parity_pos = 1 << p;
        int count = 0;

        for (int pos = 1; pos <= total; pos++)
        {
            if (pos & parity_pos)
            {
                if (hamming[pos] == '1')
                {
                    count++;
                }
            }
        }

        int parity_result;

        // Even parity
        if (count % 2 == 0)
            parity_result = 0;
        else
            parity_result = 1;

        printf("P%d check = %d\n",
               parity_pos,
               parity_result);

        /*
            If parity check is 1,
            this parity bit contributes to error position.
        */

        if (parity_result == 1)
        {
            error_position += parity_pos;
        }
    }

    printf("\n");

    if (error_position == 0)
    {
        printf("No error detected.\n");
    }
    else
    {
        printf("Error detected at position: %d\n",
               error_position);

        // Correct the error
        if (hamming[error_position] == '0')
            hamming[error_position] = '1';
        else
            hamming[error_position] = '0';

        printf("Corrected Hamming Code: ");

        for (int pos = total; pos >= 1; pos--)
            printf("%c", hamming[pos]);

        printf("\n");
    }

    close(client_fd);
    close(server_fd);

    return 0;
}
