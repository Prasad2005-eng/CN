#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define PORT 5002
#define BUF_SIZE 128
#define TIMEOUT_MS 2000

typedef struct { int seq; char data[BUF_SIZE]; } Packet;
typedef struct { int seq; } Ack;

static double now_ms(void) {
    struct timeval tv; gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

int main(void) {
    int sock, n, lost, i, attempts;
    double tt, tp, start, end;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    Packet p; Ack ack;
    struct timeval tv = {TIMEOUT_MS / 1000, (TIMEOUT_MS % 1000) * 1000};

    printf("============================================================\n");
    printf("                 STOP-AND-WAIT ARQ\n");
    printf("============================================================\n");
    printf("Enter Tt (ms): "); scanf("%lf", &tt);
    printf("Enter Tp (ms): "); scanf("%lf", &tp);
    printf("Enter Number of Packets: "); scanf("%d", &n);
    printf("Enter Lost Packet Number (0 to %d, -1 for none): ", n - 1);
    scanf("%d", &lost);

    if (tt < 0 || tp < 0 || n <= 0 || lost < -1 || lost >= n) {
        fprintf(stderr, "Invalid input.\n"); return 1;
    }

    printf("\nCALCULATION\n------------------------------------------------------------\n");
    printf("Estimated RTT = Tt + 2Tp = %.2f + 2(%.2f) = %.2f ms\n",
           tt, tp, tt + 2*tp);
    printf("Socket timeout used for ARQ = %d ms\n", TIMEOUT_MS);
    printf("A lost packet causes one timeout plus retransmission.\n");
    printf("------------------------------------------------------------\n");

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    start = now_ms();

    for (i = 0; i < n; i++) {
        p.seq = i;
        snprintf(p.data, sizeof(p.data), "ARQ-PACKET-%d", i);
        attempts = 0;

        while (1) {
            attempts++;
            printf("\nPacket %d, Attempt %d\n", i, attempts);

            if (sendto(sock, &p, sizeof(p), 0,
                       (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("sendto"); close(sock); return 1;
            }

            if (recvfrom(sock, &ack, sizeof(ack), 0,
                         (struct sockaddr *)&addr, &len) >= 0) {
                if (ack.seq == i) {
                    printf("ACK %d received -> SUCCESS\n", ack.seq);
                    break;
                }
            } else {
                printf("*** TIMEOUT waiting for ACK %d ***\n", i);
                printf("Retransmitting Packet %d...\n", i);
            }
        }
    }

    end = now_ms();
    close(sock);

    printf("\n============================================================\n");
    printf("ARQ FINAL RESULT\n");
    printf("------------------------------------------------------------\n");
    printf("Packets              : %d\n", n);
    printf("Specified lost packet: %d\n", lost);
    printf("Theoretical base time: %.2f ms (N x (Tt + 2Tp))\n",
           n * (tt + 2*tp));
    printf("Actual elapsed time  : %.2f ms\n", end - start);
    printf("============================================================\n");
    return 0;
}

