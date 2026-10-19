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
    int sock;
    struct sockaddr_in server_addr;

    char data[100];
    char key[] = "1101";
    char appended[100];
    char remainder[20];
    char transmitted[100];

    printf("Enter data stream: ");
    scanf("%s", data);

    // Validate input
    for (int i = 0; data[i] != '\0'; i++)
    {
        if (data[i] != '0' && data[i] != '1')
        {
            printf("Invalid data. Enter only 0 and 1.\n");
            return 1;
        }
    }

    int data_len = strlen(data);
    int key_len = strlen(key);

    // Append n-1 zeros
    strcpy(appended, data);

    for (int i = 0; i < key_len - 1; i++)
    {
        appended[data_len + i] = '0';
    }

    appended[data_len + key_len - 1] = '\0';

    // Calculate CRC remainder
    divide(appended, key, remainder);

    // Append remainder to original data
    strcpy(transmitted, data);
    strcat(transmitted, remainder);

    printf("\n----- CRC SENDER -----\n");
    printf("Key             : %s\n", key);
    printf("Original Data   : %s\n", data);
    printf("Appended Data   : %s\n", appended);
    printf("CRC Remainder   : %s\n", remainder);
    printf("Transmitted Data: %s\n", transmitted);

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(sock,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    // Send transmitted data
    send(sock, transmitted, strlen(transmitted) + 1, 0);

    printf("\nData sent successfully to server.\n");

    close(sock);

    return 0;
}
