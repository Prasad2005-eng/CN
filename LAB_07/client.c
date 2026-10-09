#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8087

int main()
{
    int sock;
    struct sockaddr_in server;

    char ip[50];
    char result[1500];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        printf("Socket creation failed.\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connection failed. Start the server first.\n");
        close(sock);
        return 1;
    }

    printf("Connected to Network Layer Server.\n");

    printf("Enter IP Address: ");
    scanf("%49s", ip);

    send(sock, ip, sizeof(ip), 0);

    recv(sock, result, sizeof(result), 0);

    printf("\n%s\n", result);

    close(sock);

    return 0;
}
