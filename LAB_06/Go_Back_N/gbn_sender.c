#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define PORT 5004
#define BUF_SIZE 128
#define TIMEOUT_MS 1500
typedef struct { int seq; char data[BUF_SIZE]; } Packet;
typedef struct { int ack; } Ack;

int main(void) {
    int sock, n, ws, lost, base = 0, next, j;
    double tt, tp;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    struct timeval tv = {1, 500000};
    Packet p; Ack ack;

    printf("============================================================\n");
    printf("                    GO-BACK-N ARQ\n");
    printf("============================================================\n");
    printf("Enter Tt (ms): "); scanf("%lf", &tt);
    printf("Enter Tp (ms): "); scanf("%lf", &tp);
    printf("Enter Number of Packets: "); scanf("%d", &n);
    printf("Enter Window Size: "); scanf("%d", &ws);
    printf("Enter Lost Packet Number (0 to %d): ", n-1); scanf("%d", &lost);

    if (n <= 0 || ws <= 0 || ws > n || lost < 0 || lost >= n) {
        fprintf(stderr, "Invalid input.\n"); return 1;
    }

    printf("\nCALCULATION\n------------------------------------------------------------\n");
    printf("RTT model = Tt + 2Tp = %.2f + 2(%.2f) = %.2f ms\n",
           tt, tp, tt + 2*tp);
    printf("Window = %d packets\n", ws);
    printf("When Packet %d is lost, packets after it in the current\n", lost);
    printf("window are retransmitted (Go-Back-N behavior).\n");
    printf("------------------------------------------------------------\n");

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    while (base < n) {
        printf("\nSender Window: [ ");
        for (j = base; j < base + ws && j < n; j++) printf("%d ", j);
        printf("]\n");

        for (next = base; next < base + ws && next < n; next++) {
            p.seq = next;
            snprintf(p.data, sizeof(p.data), "GBN-PACKET-%d", next);
            printf("Sending Packet %d...\n", next);
            sendto(sock, &p, sizeof(p), 0, (struct sockaddr *)&addr, sizeof(addr));
        }

        if (recvfrom(sock, &ack, sizeof(ack), 0,
                     (struct sockaddr *)&addr, &len) < 0) {
            printf("*** TIMEOUT: Go-Back-N retransmission ***\n");
            printf("Retransmitting from Packet %d\n", base);
            continue;
        }

        printf("ACK %d received.\n", ack.ack);
        if (ack.ack >= base)
            base = ack.ack + 1;
    }

    close(sock);
    printf("\n============================================================\n");
    printf("GBN COMPLETE\n");
    printf("Lost Packet = %d\n", lost);
    printf("Base RTT estimate = %.2f ms\n", tt + 2*tp);
    printf("============================================================\n");
    return 0;
}

