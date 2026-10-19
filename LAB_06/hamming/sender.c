#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main()
{
    int sock;
    struct sockaddr_in server_addr;

    char data[100];
    char hamming[100];
    int error_pos;

    printf("Enter data bits: ");
    scanf("%s", data);

    int m = strlen(data);

    // Find number of parity bits
    int r = 0;

    while ((1 << r) < (m + r + 1))
    {
        r++;
    }

    int total = m + r;

    /*
        Positions are counted from RIGHT to LEFT.

        Example for 7 data bits:

        Position:  11 10 9 8 7 6 5 4 3 2 1
                    D  D D P8 D D D P4 D P2 P1
    */

    // Initialize everything to 0
    for (int i = 1; i <= total; i++)
    {
        hamming[i] = '0';
    }

    // Put data bits into non-parity positions
    int j = 0;

    for (int pos = total; pos >= 1; pos--)
    {
        // Check if position is a parity position
        if ((pos & (pos - 1)) != 0)
        {
            hamming[pos] = data[j];
            j++;
        }
    }

    // Calculate parity bits
    for (int p = 0; p < r; p++)
    {
        int parity_pos = 1 << p;
        int count = 0;

        for (int pos = 1; pos <= total; pos++)
        {
            if (pos & parity_pos)
            {
                if (pos != parity_pos && hamming[pos] == '1')
                {
                    count++;
                }
            }
        }

        // Even parity
        if (count % 2 == 0)
            hamming[parity_pos] = '0';
        else
            hamming[parity_pos] = '1';
    }

    printf("\n----- HAMMING CODE GENERATION -----\n");

    printf("Number of data bits   : %d\n", m);
    printf("Number of parity bits : %d\n", r);

    for (int p = 0; p < r; p++)
    {
        int parity_pos = 1 << p;
        printf("P%d = %c\n",
               parity_pos,
               hamming[parity_pos]);
    }

    printf("\nPosition: ");

    for (int pos = total; pos >= 1; pos--)
        printf("%2d ", pos);

    printf("\nValue:    ");

    for (int pos = total; pos >= 1; pos--)
        printf("%2c ", hamming[pos]);

    printf("\n");

    printf("\nHamming Code: ");

    for (int pos = total; pos >= 1; pos--)
        printf("%c", hamming[pos]);

    printf("\n");

    // Optional error injection
    printf("\nEnter error position (0 for no error): ");
    scanf("%d", &error_pos);

    if (error_pos > 0 && error_pos <= total)
    {
        if (hamming[error_pos] == '0')
            hamming[error_pos] = '1';
        else
            hamming[error_pos] = '0';

        printf("\nBit at position %d has been flipped.\n",
               error_pos);

        printf("Code sent to server: ");

        for (int pos = total; pos >= 1; pos--)
            printf("%c", hamming[pos]);

        printf("\n");
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect
    if (connect(sock,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    // Send total number of bits
    send(sock, &total, sizeof(total), 0);

    // Send Hamming code
    send(sock, hamming + 1, total, 0);

    printf("\nHamming code sent successfully.\n");

    close(sock);

    return 0;
}
