#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define PORT 5001
#define BUF_SIZE 128

typedef struct {
    int seq;
    char data[BUF_SIZE];
} Packet;

typedef struct {
    int seq;
} Ack;

static double now_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

int main(void) {
    int sock, n, i;
    double tt, tp, theoretical, start, end;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    Packet p;
    Ack ack;

    printf("============================================================\n");
    printf("                 SIMPLE STOP-AND-WAIT\n");
    printf("============================================================\n");
    printf("Enter Transmission Time Tt (ms): ");
    scanf("%lf", &tt);
    printf("Enter Propagation Time Tp (ms): ");
    scanf("%lf", &tp);
    printf("Enter Number of Packets: ");
    scanf("%d", &n);

    if (tt < 0 || tp < 0 || n <= 0) {
        fprintf(stderr, "Invalid input.\n");
        return 1;
    }

    theoretical = n * (tt + 2.0 * tp);

    printf("\nCALCULATION\n");
    printf("------------------------------------------------------------\n");
    printf("Assumption: ACK transmission time is negligible.\n");
    printf("Time per packet = Tt + 2Tp\n");
    printf("                = %.2f + 2(%.2f)\n", tt, tp);
    printf("                = %.2f ms\n", tt + 2.0 * tp);
    printf("Total theoretical time = N(Tt + 2Tp)\n");
    printf("                      = %d(%.2f)\n", n, tt + 2.0 * tp);
    printf("                      = %.2f ms\n", theoretical);
    printf("------------------------------------------------------------\n");

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    start = now_ms();

    for (i = 0; i < n; i++) {
        p.seq = i;
        snprintf(p.data, sizeof(p.data), "DATA-PACKET-%d", i);

        printf("\nPacket %d\n", i);
        printf("  Tt = %.2f ms, Tp = %.2f ms\n", tt, tp);
        printf("  Sending %s...\n", p.data);

        if (sendto(sock, &p, sizeof(p), 0,
                   (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("sendto"); close(sock); return 1;
        }

        if (recvfrom(sock, &ack, sizeof(ack), 0,
                     (struct sockaddr *)&addr, &len) < 0) {
            perror("recvfrom"); close(sock); return 1;
        }

        if (ack.seq == i)
            printf("  ACK %d received -> SUCCESS\n", ack.seq);
        else
            printf("  Unexpected ACK %d\n", ack.seq);
    }

    end = now_ms();
    close(sock);

    printf("\n============================================================\n");
    printf("FINAL RESULT\n");
    printf("------------------------------------------------------------\n");
    printf("Packets Sent              : %d\n", n);
    printf("Time per packet           : %.2f ms\n", tt + 2.0 * tp);
    printf("Total Theoretical Time    : %.2f ms\n", theoretical);
    printf("Actual Program Elapsed    : %.2f ms\n", end - start);
    printf("============================================================\n");
    return 0;
}

