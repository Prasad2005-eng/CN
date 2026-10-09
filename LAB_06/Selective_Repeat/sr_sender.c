#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define PORT 5005
#define BUF_SIZE 128
typedef struct { int seq; char data[BUF_SIZE]; } Packet;
typedef struct { int ack; } Ack;

int main(void) {
    int sock, n, ws, lost, i, received[512] = {0};
    double tt, tp;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    struct timeval tv = {1, 500000};
    Packet p; Ack ack;

    printf("============================================================\n");
    printf("                  SELECTIVE REPEAT ARQ\n");
    printf("============================================================\n");
    printf("Enter Tt (ms): "); scanf("%lf", &tt);
    printf("Enter Tp (ms): "); scanf("%lf", &tp);
    printf("Enter Number of Packets (<=500): "); scanf("%d", &n);
    printf("Enter Window Size: "); scanf("%d", &ws);
    printf("Enter Lost Packet Number (0 to %d): ", n-1); scanf("%d", &lost);

    if (n <= 0 || n > 500 || ws <= 0 || ws > n || lost < 0 || lost >= n) {
        fprintf(stderr, "Invalid input.\n"); return 1;
    }

    printf("\nCALCULATION\n------------------------------------------------------------\n");
    printf("RTT model = Tt + 2Tp = %.2f + 2(%.2f) = %.2f ms\n",
           tt, tp, tt + 2*tp);
    printf("Sender Window (SW) = %d\n", ws);
    printf("Receiver Window (RW) = %d\n", ws);
    printf("Selective Repeat retransmits ONLY the packet that\n");
    printf("does not receive its ACK.\n");
    printf("------------------------------------------------------------\n");

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    for (i = 0; i < n; i += ws) {
        int end = i + ws;
        if (end > n) end = n;

        printf("\nSW = [ ");
        for (int j = i; j < end; j++) printf("%d ", j);
        printf("]\n");

        for (int j = i; j < end; j++) {
            p.seq = j;
            snprintf(p.data, sizeof(p.data), "SR-PACKET-%d", j);
            printf("Sending Packet %d...\n", j);
            sendto(sock, &p, sizeof(p), 0, (struct sockaddr *)&addr, sizeof(addr));
        }

        int acks = 0;
        while (acks < end - i) {
            if (recvfrom(sock, &ack, sizeof(ack), 0,
                         (struct sockaddr *)&addr, &len) >= 0) {
                if (ack.ack >= i && ack.ack < end && !received[ack.ack]) {
                    received[ack.ack] = 1;
                    acks++;
                    printf("ACK %d received.\n", ack.ack);
                }
            } else {
                printf("Timeout: retransmitting unACKed packets only.\n");
                for (int j = i; j < end; j++) {
                    if (!received[j]) {
                        p.seq = j;
                        snprintf(p.data, sizeof(p.data), "SR-RETX-%d", j);
                        printf("Retransmitting ONLY Packet %d...\n", j);
                        sendto(sock, &p, sizeof(p), 0,
                               (struct sockaddr *)&addr, sizeof(addr));
                    }
                }
            }
        }
    }

    close(sock);
    printf("\n============================================================\n");
    printf("SELECTIVE REPEAT COMPLETE\n");
    printf("Lost Packet          : %d\n", lost);
    printf("SW                   : %d\n", ws);
    printf("RW                   : %d\n", ws);
    printf("Base RTT estimate    : %.2f ms\n", tt + 2*tp);
    printf("Only unacknowledged packets are retransmitted.\n");
    printf("============================================================\n");
    return 0;
}
