#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

// XOR division
void divide(char data[], char key[], char remainder[])
{
    int data_len = strlen(data);
    int key_len = strlen(key);

    char temp[100];
    strcpy(temp, data);

    for (int i = 0; i <= data_len - key_len; i++)
    {
        if (temp[i] == '1')
        {
            for (int j = 0; j < key_len; j++)
            {
                temp[i + j] =
                    (temp[i + j] == key[j]) ? '0' : '1';
            }
        }
    }

    // Last key_len - 1 bits are the remainder
    strncpy(remainder,
            temp + data_len - key_len + 1,
            key_len - 1);

    remainder[key_len - 1] = '\0';
}

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;

    char received[100];
    char key[] = "1101";
    char remainder[20];

    socklen_t addr_size = sizeof(client_addr);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    // Listen
    listen(server_fd, 5);

    printf("Server waiting for connection...\n");

    // Accept client
    client_fd = accept(server_fd,
                       (struct sockaddr *)&client_addr,
                       &addr_size);

    if (client_fd < 0)
    {
        perror("Accept failed");
        close(server_fd);
        return 1;
    }

    // Receive transmitted data
    recv(client_fd, received, sizeof(received), 0);

    printf("\n----- CRC RECEIVER -----\n");

    printf("Key             : %s\n", key);
    printf("Received Data   : %s\n", received);

    // Divide received codeword by key
    divide(received, key, remainder);

    printf("CRC Remainder   : %s\n", remainder);

    // Check whether remainder is all zeros
    int error = 0;

    for (int i = 0; remainder[i] != '\0'; i++)
    {
        if (remainder[i] != '0')
        {
            error = 1;
            break;
        }
    }

    if (error == 0)
    {
        printf("NO ERROR DETECTED\n");
    }
    else
    {
        printf("ERROR DETECTED IN DATA\n");
    }

    close(client_fd);
    close(server_fd);

    return 0;
}
