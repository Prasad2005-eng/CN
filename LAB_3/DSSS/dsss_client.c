#include "dsss_common.h"

// XOR two binary strings
void xorBits(char a[], char b[], char result[])
{
    for(int i = 0; i < 4; i++)
    {
        if(a[i] == b[i])
            result[i] = '0';
        else
            result[i] = '1';
    }

    result[4] = '\0';
}

int main()
{
    char bits[101];

    printf("Enter binary data: ");
    scanf("%100s", bits);

    // Validate input
    for(int i = 0; bits[i] != '\0'; i++)
    {
        if(bits[i] != '0' && bits[i] != '1')
        {
            printf("Invalid input. Enter only 0 and 1.\n");
            return 1;
        }
    }

    // Create socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server;

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    inet_pton(
        AF_INET,
        "127.0.0.1",
        &server.sin_addr
    );

    printf("\nPN Code: %s\n\n", pnCode);

    // --------------------------------
    // Spread and send each bit
    // --------------------------------
    for(int i = 0; bits[i] != '\0'; i++)
    {
        Packet p;

        p.seqNo = i;

        char repeatedBit[5];
        char spreadData[5];

        // Convert one bit into 4 bits
        if(bits[i] == '1')
            strcpy(repeatedBit, "1111");
        else
            strcpy(repeatedBit, "0000");

        // XOR with PN sequence
        xorBits(
            repeatedBit,
            pnCode,
            spreadData
        );

        strcpy(p.data, spreadData);

        // Send packet
        sendto(
            sock,
            &p,
            sizeof(p),
            0,
            (struct sockaddr *)&server,
            sizeof(server)
        );

        printf(
            "Packet %d | Bit = %c | "
            "XOR Result = %s\n",
            i + 1,
            bits[i],
            p.data
        );

        sleep(1);
    }

    // END packet
    Packet end;

    end.seqNo = END;
    strcpy(end.data, "");

    sendto(
        sock,
        &end,
        sizeof(end),
        0,
        (struct sockaddr *)&server,
        sizeof(server)
    );

    printf("\nAll packets sent.\n");

    close(sock);

    return 0;
}
