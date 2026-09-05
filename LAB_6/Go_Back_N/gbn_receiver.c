#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5004
#define BUF_SIZE 128
typedef struct { int seq; char data[BUF_SIZE]; } Packet;
typedef struct { int ack; } Ack;

int main(void) {
    int sock, lost, expected = 0, loss_done = 0;
    struct sockaddr_in addr, client;
    socklen_t len = sizeof(client);
    Packet p; Ack ack;

    printf("============================================================\n");
    printf("                 GO-BACK-N RECEIVER\n");
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

        if (p.seq == expected) {
            printf("Packet %d accepted in order.\n", p.seq);
            ack.ack = p.seq;
            expected++;
            sendto(sock, &ack, sizeof(ack), 0,
                   (struct sockaddr *)&client, len);
            printf("ACK %d sent.\n", ack.ack);
        } else if (p.seq > expected) {
            printf("Packet %d OUT OF ORDER; expected %d.\n", p.seq, expected);
            ack.ack = expected - 1;
            sendto(sock, &ack, sizeof(ack), 0,
                   (struct sockaddr *)&client, len);
            printf("Duplicate/previous ACK %d sent.\n", ack.ack);
        } else {
            ack.ack = p.seq;
            sendto(sock, &ack, sizeof(ack), 0,
                   (struct sockaddr *)&client, len);
        }
    }
}

