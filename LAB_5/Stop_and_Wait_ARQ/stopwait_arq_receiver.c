#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5002
#define BUF_SIZE 128

typedef struct { int seq; char data[BUF_SIZE]; } Packet;
typedef struct { int seq; } Ack;

int main(void) {
    int sock, lost, loss_done = 0;
    struct sockaddr_in addr, client;
    socklen_t len = sizeof(client);
    Packet p; Ack ack;

    printf("============================================================\n");
    printf("              STOP-AND-WAIT ARQ RECEIVER\n");
    printf("============================================================\n");
    printf("Enter Lost Packet Number (-1 for none): ");
    scanf("%d", &lost);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

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

        printf("\nReceived Packet %d: %s\n", p.seq, p.data);

        if (p.seq == lost && !loss_done) {
            printf("*** Simulating LOSS of Packet %d ***\n", p.seq);
            loss_done = 1;
            continue;
        }

        ack.seq = p.seq;
        sendto(sock, &ack, sizeof(ack), 0,
               (struct sockaddr *)&client, len);
        printf("ACK %d sent.\n", ack.seq);
    }
}

