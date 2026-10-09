#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5005
#define BUF_SIZE 128
typedef struct { int seq; char data[BUF_SIZE]; } Packet;
typedef struct { int ack; } Ack;

int main(void) {
    int sock, lost, loss_done = 0;
    int buffered[512] = {0};
    struct sockaddr_in addr, client;
    socklen_t len = sizeof(client);
    Packet p; Ack ack;

    printf("============================================================\n");
    printf("              SELECTIVE REPEAT RECEIVER\n");
    printf("============================================================\n");
    printf("Enter Lost Packet Number (-1 for none): ");
    scanf("%d", &lost);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(sock); return 1;
    }

    printf("Listening on port %d...\n", PORT);

    while (1) {
        if (recvfrom(sock, &p, sizeof(p), 0,
                     (struct sockaddr *)&client, &len) < 0) {
            perror("recvfrom"); close(sock); return 1;
        }

        printf("\nReceived Packet %d\n", p.seq);

        if (p.seq == lost && !loss_done) {
            printf("*** Packet %d LOST ***\n", p.seq);
            loss_done = 1;
            continue;
        }

        if (p.seq >= 0 && p.seq < 512)
            buffered[p.seq] = 1;

        printf("Packet %d accepted/buffered.\n", p.seq);
        ack.ack = p.seq;
        sendto(sock, &ack, sizeof(ack), 0,
               (struct sockaddr *)&client, len);
        printf("Individual ACK %d sent.\n", ack.ack);
    }
}

