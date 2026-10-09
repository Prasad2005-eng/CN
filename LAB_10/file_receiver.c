#include "file_transfer.h"

int main(void)
{
    int sockfd;
    struct sockaddr_in server, sender;
    socklen_t senderLen = sizeof(sender);

    FilePacket packet;
    AckPacket ack;

    FILE *fp = NULL;
    char outputPath[512] = "";
    uint64_t expectedSize = 0;
    uint64_t receivedSize = 0;
    int expectedSeq = 1;
    int transferStarted = 0;

    mkdir("received_files", 0755);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&server,
             sizeof(server)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    // One-second timeout is also used while waiting for duplicate ENDs.
    struct timeval timeout = {1, 0};

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
               &timeout, sizeof(timeout));

    printf("UDP file receiver listening on port %d...\n", PORT);
    printf("Waiting for a file...\n");

    while (1)
    {
        senderLen = sizeof(sender);

        ssize_t n = recvfrom(
            sockfd, &packet, sizeof(packet), 0,
            (struct sockaddr *)&sender, &senderLen
        );

        if (n < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;

            perror("recvfrom");
            break;
        }

        if (n != sizeof(packet))
        {
            printf("Invalid packet received.\n");
            continue;
        }

        if (packet.type == START_PACKET)
        {
            // Only accept a simple filename, not a path.
            if (strchr(packet.filename, '/') != NULL ||
                strchr(packet.filename, '\\') != NULL ||
                strstr(packet.filename, "..") != NULL ||
                packet.filename[0] == '\0' ||
                packet.fileSize > 1000000000ULL)
            {
                printf("Invalid filename or file too large.\n");
                continue;
            }

            if (!transferStarted)
            {
                snprintf(outputPath, sizeof(outputPath),
                         "received_files/%s", packet.filename);

                fp = fopen(outputPath, "wb");

                if (fp == NULL)
                {
                    perror("Cannot create output file");
                    continue;
                }

                expectedSize = packet.fileSize;
                receivedSize = 0;
                expectedSeq = 1;
                transferStarted = 1;

                printf("\nReceiving file: %s\n", packet.filename);
                printf("Expected size: %llu bytes\n",
                       (unsigned long long)expectedSize);
            }

            ack.ack = 0;
            sendto(sockfd, &ack, sizeof(ack), 0,
                   (struct sockaddr *)&sender, senderLen);
        }
        else if (packet.type == DATA_PACKET && transferStarted)
        {
            if (packet.dataLen < 1 ||
                packet.dataLen > CHUNK_SIZE)
                continue;

            if (packet.seq == expectedSeq)
            {
                if (receivedSize + (uint64_t)packet.dataLen >
                    expectedSize)
                {
                    printf("Unexpected extra data; aborting.\n");
                    fclose(fp);
                    fp = NULL;
                    transferStarted = 0;
                    continue;
                }

                size_t written = fwrite(
                    packet.data, 1, packet.dataLen, fp
                );

                if (written != (size_t)packet.dataLen)
                {
                    perror("File write failed");
                    fclose(fp);
                    fp = NULL;
                    transferStarted = 0;
                    continue;
                }

                receivedSize += written;
                printf("Received packet %d (%d bytes)\n",
                       packet.seq, packet.dataLen);

                expectedSeq++;
            }

            // ACK the most recently accepted packet, including duplicates.
            if (packet.seq < expectedSeq)
            {
                ack.ack = packet.seq;

                sendto(sockfd, &ack, sizeof(ack), 0,
                       (struct sockaddr *)&sender, senderLen);
            }
        }
        else if (packet.type == END_PACKET && transferStarted)
        {
            if (packet.seq != expectedSeq ||
                receivedSize != expectedSize)
            {
                printf("End packet received, but size/sequence mismatches.\n");
                continue;
            }

            if (fclose(fp) != 0)
            {
                perror("File close failed");
                fp = NULL;
                transferStarted = 0;
                continue;
            }

            fp = NULL;
            transferStarted = 0;

            printf("\nFile saved to: %s\n", outputPath);
            printf("Received %llu of %llu bytes.\n",
                   (unsigned long long)receivedSize,
                   (unsigned long long)expectedSize);

            ack.ack = packet.seq;

            sendto(sockfd, &ack, sizeof(ack), 0,
                   (struct sockaddr *)&sender, senderLen);

            printf("Transfer completed. Waiting briefly for duplicate final packets...\n");

            /*
               Keep the receiver alive briefly so it can acknowledge
               a retransmitted END if the original ACK was lost.
            */
            for (int i = 0; i < 6; i++)
            {
                senderLen = sizeof(sender);

                ssize_t extra = recvfrom(
                    sockfd, &packet, sizeof(packet), 0,
                    (struct sockaddr *)&sender, &senderLen
                );

                if (extra == sizeof(packet) &&
                    packet.type == END_PACKET &&
                    packet.seq == expectedSeq)
                {
                    ack.ack = packet.seq;

                    sendto(sockfd, &ack, sizeof(ack), 0,
                           (struct sockaddr *)&sender, senderLen);
                }
            }

            printf("Receiver ready for another file.\n");
        }
    }

    if (fp != NULL)
        fclose(fp);

    close(sockfd);
    return 0;
}
