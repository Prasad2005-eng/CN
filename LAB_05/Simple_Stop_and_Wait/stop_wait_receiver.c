#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 5001
#define BUF_SIZE 128

typedef struct {
    int seq;
    char data[BUF_SIZE];
} Packet;

typedef struct {
    int seq;
} Ack;

int main(void) {
    int sock;
    struct sockaddr_in addr, client;
    socklen_t len = sizeof(client);
    Packet p;
    Ack ack;

    printf("============================================================\n");
    printf("             SIMPLE STOP-AND-WAIT RECEIVER\n");
    printf("============================================================\n");

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(sock); return 1;
    }

    printf("Listening on 127.0.0.1:%d ...\n", PORT);

    while (1) {
        ssize_t r = recvfrom(sock, &p, sizeof(p), 0,
                             (struct sockaddr *)&client, &len);
        if (r < 0) { perror("recvfrom"); close(sock); return 1; }

        printf("\nReceived Packet %d: %s\n", p.seq, p.data);
        ack.seq = p.seq;

        if (sendto(sock, &ack, sizeof(ack), 0,
                   (struct sockaddr *)&client, len) < 0) {
            perror("sendto"); close(sock); return 1;
        }

        printf("ACK %d sent.\n", ack.seq);
    }
}

