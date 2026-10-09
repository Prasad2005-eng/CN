#include "file_transfer.h"

static int sendReliable(int sockfd, struct sockaddr_in *receiver,
                        FilePacket *packet)
{
    socklen_t addrLen = sizeof(*receiver);

    for (int attempt = 0; attempt < MAX_RETRIES; attempt++)
    {
        if (sendto(sockfd, packet, sizeof(*packet), 0,
                   (struct sockaddr *)receiver, addrLen) < 0)
        {
            perror("sendto");
            return 0;
        }

        AckPacket ack;
        struct sockaddr_in from;
        socklen_t fromLen = sizeof(from);

        ssize_t n = recvfrom(sockfd, &ack, sizeof(ack), 0,
                             (struct sockaddr *)&from, &fromLen);

        if (n == sizeof(ack) &&
            from.sin_addr.s_addr == receiver->sin_addr.s_addr &&
            from.sin_port == receiver->sin_port &&
            ack.ack == packet->seq)
        {
            return 1;
        }

        printf("Timeout or incorrect ACK; retry %d/%d\n",
               attempt + 1, MAX_RETRIES);
    }

    return 0;
}

int main(void)
{
    int sockfd;
    FILE *fp;
    char filepath[512];
    char filename[MAX_FILENAME];

    struct sockaddr_in receiver;
    FilePacket packet;

    printf("UDP FILE TRANSFER - SENDER\n");
    printf("Enter file path to send:\n");
    printf("Examples: sample.sh, sample.txt, sample.mp3, sample.mp4\n");
    printf("File path: ");

    if (scanf("%511s", filepath) != 1)
        return 1;

    fp = fopen(filepath, "rb");

    if (fp == NULL)
    {
        perror("Cannot open file");
        return 1;
    }

    // Extract filename from the path.
    const char *base = strrchr(filepath, '/');
    base = (base == NULL) ? filepath : base + 1;

    if (strlen(base) == 0 || strlen(base) >= sizeof(filename))
    {
        printf("Invalid filename.\n");
        fclose(fp);
        return 1;
    }

    strcpy(filename, base);

    // Determine file size.
    if (fseek(fp, 0, SEEK_END) != 0)
    {
        perror("fseek");
        fclose(fp);
        return 1;
    }

    long fileSize = ftell(fp);

    if (fileSize < 0 || fseek(fp, 0, SEEK_SET) != 0)
    {
        perror("File size/seek error");
        fclose(fp);
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        fclose(fp);
        return 1;
    }

    // Wait up to one second for an ACK.
    struct timeval timeout = {1, 0};

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
               &timeout, sizeof(timeout));

    memset(&receiver, 0, sizeof(receiver));
    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(PORT);

    // Change this IP to the receiver machine's IP for two machines.
    inet_pton(AF_INET, "127.0.0.1", &receiver.sin_addr);

    memset(&packet, 0, sizeof(packet));
    packet.type = START_PACKET;
    packet.seq = 0;
    packet.fileSize = (uint64_t)fileSize;
    strcpy(packet.filename, filename);

    printf("\nSending file: %s (%ld bytes)\n", filename, fileSize);

    if (!sendReliable(sockfd, &receiver, &packet))
    {
        printf("Failed to establish transfer.\n");
        fclose(fp);
        close(sockfd);
        return 1;
    }

    int seq = 1;
    size_t bytesRead;

    while ((bytesRead = fread(packet.data, 1, CHUNK_SIZE, fp)) > 0)
    {
        packet.type = DATA_PACKET;
        packet.seq = seq++;
        packet.dataLen = (int)bytesRead;
        packet.fileSize = 0;
        packet.filename[0] = '\0';

        if (!sendReliable(sockfd, &receiver, &packet))
        {
            printf("Transfer failed at packet %d.\n", packet.seq);
            fclose(fp);
            close(sockfd);
            return 1;
        }

        printf("Sent packet %d (%zu bytes)\n",
               packet.seq, bytesRead);
    }

    if (ferror(fp))
    {
        perror("File read failed");
        fclose(fp);
        close(sockfd);
        return 1;
    }

    fclose(fp);

    memset(&packet, 0, sizeof(packet));
    packet.type = END_PACKET;
    packet.seq = seq;

    if (!sendReliable(sockfd, &receiver, &packet))
    {
        printf("Final acknowledgement not received.\n");
        close(sockfd);
        return 1;
    }

    printf("\nFile transferred successfully!\n");

    close(sockfd);
    return 0;
}
