#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8087

int main()
{
    int server_fd, client_socket;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);

    char ip[50];
    char result[1500];

    int a, b, c, d;
    int nidBits, hidBits;
    int networks, hosts;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        printf("Socket creation failed.\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Bind failed.\n");
        close(server_fd);
        return 1;
    }

    listen(server_fd, 5);

    printf("Network Layer Server Started...\n");
    printf("Waiting for client connection...\n");

    client_socket = accept(server_fd,
                           (struct sockaddr *)&client,
                           &client_len);

    if (client_socket < 0)
    {
        printf("Client connection failed.\n");
        close(server_fd);
        return 1;
    }

    printf("Client connected successfully.\n");

    recv(client_socket, ip, sizeof(ip), 0);

    sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d);

    if (a >= 1 && a <= 126)
    {
        nidBits = 8;
        hidBits = 24;
        networks = 126;
        hosts = 16777214;

        sprintf(result,
                "========== IP ANALYSIS ==========\n"
                "IP Address          : %s\n"
                "Class               : A\n"
                "NID Bits            : %d\n"
                "HID Bits            : %d\n"
                "Number of Networks  : %d\n"
                "Number of Hosts     : %d\n"
                "IP Range            : 1.0.0.0 - 126.255.255.255\n"
                "LBA                 : %d.0.0.1\n"
                "DBA                 : %d.255.255.254",
                ip, nidBits, hidBits, networks, hosts, a, a);
    }
    else if (a >= 128 && a <= 191)
    {
        nidBits = 16;
        hidBits = 16;
        networks = 16384;
        hosts = 65534;

        sprintf(result,
                "========== IP ANALYSIS ==========\n"
                "IP Address          : %s\n"
                "Class               : B\n"
                "NID Bits            : %d\n"
                "HID Bits            : %d\n"
                "Number of Networks  : %d\n"
                "Number of Hosts     : %d\n"
                "IP Range            : 128.0.0.0 - 191.255.255.255\n"
                "LBA                 : %d.%d.0.1\n"
                "DBA                 : %d.%d.255.254",
                ip, nidBits, hidBits, networks, hosts,
                a, b, a, b);
    }
    else if (a >= 192 && a <= 223)
    {
        nidBits = 24;
        hidBits = 8;
        networks = 16777216;
        hosts = 254;

        sprintf(result,
                "========== IP ANALYSIS ==========\n"
                "IP Address          : %s\n"
                "Class               : C\n"
                "NID Bits            : %d\n"
                "HID Bits            : %d\n"
                "Number of Networks  : %d\n"
                "Number of Hosts     : %d\n"
                "IP Range            : 192.0.0.0 - 223.255.255.255\n"
                "LBA                 : %d.%d.%d.1\n"
                "DBA                 : %d.%d.%d.255",
                ip, nidBits, hidBits, networks, hosts,
                a, b, c, a, b, c);
    }
    else if (a >= 224 && a <= 239)
    {
        sprintf(result,
                "========== IP ANALYSIS ==========\n"
                "IP Address          : %s\n"
                "Class               : D\n"
                "Used for Multicasting",
                ip);
    }
    else if (a >= 240 && a <= 255)
    {
        sprintf(result,
                "========== IP ANALYSIS ==========\n"
                "IP Address          : %s\n"
                "Class               : E\n"
                "Used for Experimental/Reserved purpose",
                ip);
    }
    else
    {
        sprintf(result, "Invalid IP Address");
    }

    send(client_socket, result, sizeof(result), 0);

    printf("\n%s\n", result);

    close(client_socket);
    close(server_fd);

    return 0;
}
