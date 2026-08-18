#include "fhss_common.h"

int main()
{
    int sock;

    struct sockaddr_in server;
    struct sockaddr_in client;

    socklen_t clientLen = sizeof(client);

    // --------------------------------
    // Create UDP socket
    // --------------------------------
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

    // --------------------------------
    // Bind
    // --------------------------------
    if(bind(
        sock,
        (struct sockaddr *)&server,
        sizeof(server)) < 0)
    {
        perror("Bind failed");
        return 1;
    }

    printf("FHSS Server listening on port %d...\n\n", PORT);

    // Store received packets
    char chunks[MAX_PACKETS][4];

    int count = 0;

    Packet p;

    // --------------------------------
    // Receive packets
    // --------------------------------
    while(1)
    {
        int bytes = recvfrom(
            sock,
            &p,
            sizeof(p),
            0,
            (struct sockaddr *)&client,
            &clientLen
        );

        if(bytes < 0)
        {
            perror("recvfrom failed");
            break;
        }

        // Check END packet
        if(p.seqNo == END)
        {
            printf("END packet received.\n");
            break;
        }

        // Validate sequence number
        if(p.seqNo < 0 || p.seqNo >= MAX_PACKETS)
        {
            printf("Invalid sequence number.\n");
            continue;
        }

        // --------------------------------
        // Calculate expected frequency
        // --------------------------------
        int expectedFrequency =
            freqBands[hopSequence[p.seqNo % 8]];

        // Check frequency
        if(p.frequency != expectedFrequency)
        {
            printf(
                "Packet %d rejected: "
                "Wrong frequency!\n",
                p.seqNo
            );

            continue;
        }

        // Store packet
        strcpy(chunks[p.seqNo], p.data);

        if(p.seqNo + 1 > count)
            count = p.seqNo + 1;

        printf(
            "Received Packet %d | Seq = %d | "
            "Frequency = %d Hz | Data = %s\n",
            p.seqNo + 1,
            p.seqNo,
            p.frequency,
            p.data
        );
    }

    // --------------------------------
    // Reassemble spread data
    // --------------------------------
    char spread[301] = "";

    for(int i = 0; i < count; i++)
    {
        strcat(spread, chunks[i]);
    }

    printf("\nReassembled Spread Data: %s\n", spread);

    // --------------------------------
    // Despread
    // --------------------------------
    char original[101] = "";

    for(int i = 0; i < strlen(spread); i += 3)
    {
        if(strncmp(spread + i, "111", 3) == 0)
            strcat(original, "1");
        else
            strcat(original, "0");
    }

    printf("Recovered Original Data: %s\n", original);

    close(sock);

    return 0;
}
