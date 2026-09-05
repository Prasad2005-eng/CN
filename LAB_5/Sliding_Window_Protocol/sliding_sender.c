#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5003
#define BUF_SIZE 128
typedef struct { int seq; char data[BUF_SIZE]; } Packet;
typedef struct { int ack; } Ack;

int main(void) {
    int sock, n, ws, bits, i;
    double tt, tp, a, tao, total;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    Packet p; Ack ack;

    printf("============================================================\n");
    printf("              SLIDING WINDOW - NOISELESS\n");
    printf("============================================================\n");
    printf("Enter Tt (ms): "); scanf("%lf", &tt);
    printf("Enter Tp (ms): "); scanf("%lf", &tp);
    printf("Enter Number of Packets: "); scanf("%d", &n);

    if (tt <= 0 || tp < 0 || n <= 0) {
        fprintf(stderr, "Invalid input.\n"); return 1;
    }

    bits = 0;
    while ((1 << bits) < n) bits++;
    if (bits == 0) bits = 1;
    int seq_space = 1 << bits;
    ws = seq_space - 1;  /* noiseless sliding window convention */
    if (ws > n) ws = n;

    a = tp / tt;
    tao = tt + 2.0 * tp;
    total = n * (tt + 2.0 * tp);

    printf("\nCALCULATIONS\n------------------------------------------------------------\n");
    printf("a = Tp/Tt = %.2f/%.2f = %.4f\n", tp, tt, a);
    printf("2^m >= N -> 2^m >= %d -> m = %d bits\n", n, bits);
    printf("Sequence number space = 2^m = %d\n", seq_space);
    printf("Ws = 2^m - 1 = %d\n", seq_space - 1);
    printf("Effective Ws used = %d\n", ws);
    printf("TAO (cycle model) = Tt + 2Tp = %.2f ms\n", tao);
    printf("Total theoretical time = N(Tt + 2Tp) = %.2f ms\n", total);
    printf("------------------------------------------------------------\n");

    printf("\nWINDOW ARRAY DEMONSTRATION\n");
    for (i = 0; i < n; i++) {
        int j;
        printf("Window: [ ");
        for (j = i; j < i + ws && j < n; j++)
            printf("%d ", j % seq_space);
        printf("]\n");
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    printf("\nTRANSMISSION\n");
    for (i = 0; i < n; i++) {
        p.seq = i % seq_space;
        snprintf(p.data, sizeof(p.data), "SW-PACKET-%d", i);
        printf("Sending Packet %d (Seq=%d)...\n", i, p.seq);
        sendto(sock, &p, sizeof(p), 0, (struct sockaddr *)&addr, sizeof(addr));
        if (recvfrom(sock, &ack, sizeof(ack), 0,
                     (struct sockaddr *)&addr, &len) >= 0)
            printf("  ACK %d received.\n", ack.ack);
    }

    close(sock);
    printf("\n============================================================\n");
    printf("SUMMARY\n");
    printf("Maximum sequence numbers : %d\n", seq_space);
    printf("Sequence number bits     : %d\n", bits);
    printf("Window Size (Ws)         : %d\n", ws);
    printf("TAO                      : %.2f ms\n", tao);
    printf("Total theoretical time   : %.2f ms\n", total);
    printf("============================================================\n");
    return 0;
}

