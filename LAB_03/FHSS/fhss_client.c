#include "fhss_common.h"

int main()
{
    char bits[101];
    char spread[301] = "";

    // --------------------------------
    // Read input
    // --------------------------------
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

    // --------------------------------
    // Spreading
    // 1 -> 111
    // 0 -> 000
    // --------------------------------
    for(int i = 0; bits[i] != '\0'; i++)
    {
        if(bits[i] == '1')
            strcat(spread, "111");
        else
            strcat(spread, "000");
    }

    printf("\nOriginal Data : %s", bits);
    printf("\nSpread Data   : %s\n\n", spread);

    // --------------------------------
    // Create UDP socket
    // --------------------------------
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

    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    // Number of packets
    int numPackets = strlen(spread) / 3;

    // --------------------------------
    // Send packets
    // --------------------------------
    for(int i = 0; i < numPackets; i++)
    {
        Packet p;

        // Sequence number
        p.seqNo = i;

        // Frequency hopping
        p.frequency =
            freqBands[hopSequence[i % 8]];

        // Copy 3 bits
        memcpy(p.data, spread + (i * 3), 3);
        p.data[3] = '\0';

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
            "Sending Packet %d | Seq = %d | "
            "Frequency = %d Hz | Data = %s\n",
            i + 1,
            p.seqNo,
            p.frequency,
            p.data
        );

        sleep(1);
    }

    // --------------------------------
    // Send END packet
    // --------------------------------
    Packet end;

    end.seqNo = END;
    end.frequency = 0;
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
