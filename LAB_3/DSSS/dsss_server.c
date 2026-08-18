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
    int sock;

    struct sockaddr_in server;
    struct sockaddr_in client;

    socklen_t clientLen = sizeof(client);

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind
    if(bind(
        sock,
        (struct sockaddr *)&server,
        sizeof(server)) < 0)
    {
        perror("Bind failed");
        return 1;
    }

    printf("DSSS Server listening on port %d...\n", PORT);
    printf("PN Code: %s\n\n", pnCode);

    char packets[MAX_PACKETS][5];

    int count = 0;

    Packet p;

    // --------------------------------
    // Receive packets
    // --------------------------------
    while(1)
    {
        recvfrom(
            sock,
            &p,
            sizeof(p),
            0,
            (struct sockaddr *)&client,
            &clientLen
        );

        if(p.seqNo == END)
        {
            printf("END packet received.\n");
            break;
        }

        if(p.seqNo < 0 || p.seqNo >= MAX_PACKETS)
        {
            printf("Invalid sequence number.\n");
            continue;
        }

        strcpy(
            packets[p.seqNo],
            p.data
        );

        if(p.seqNo + 1 > count)
            count = p.seqNo + 1;

        printf(
            "Received Packet %d | "
            "Spread Data = %s\n",
            p.seqNo + 1,
            p.data
        );
    }

    // --------------------------------
    // Despread
    // --------------------------------
    char original[101] = "";

    for(int i = 0; i < count; i++)
    {
        char result[5];

        // XOR received data with same PN code
        xorBits(
            packets[i],
            pnCode,
            result
        );

        /*
           result will be:

           1111 -> original 1
           0000 -> original 0
        */

        if(strcmp(result, "1111") == 0)
            strcat(original, "1");
        else
            strcat(original, "0");

        printf(
            "Packet %d | Received = %s | "
            "After XOR = %s\n",
            i + 1,
            packets[i],
            result
        );
    }

    printf(
        "\nRecovered Original Data: %s\n",
        original
    );

    close(sock);

    return 0;
}
