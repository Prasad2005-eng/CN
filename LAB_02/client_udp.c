#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>

int main()
{
    // Create Socket
    int clientsocket;
    char serverresponse[256];

    struct sockaddr_in serveraddress;

    clientsocket = socket(AF_INET, SOCK_DGRAM, 0);

    if(clientsocket < 0)
        printf("Socket is not created\n");
    else
        printf("Socket is created successfully\n");

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(9000);
    serveraddress.sin_addr.s_addr = INADDR_ANY;

    // Send message to server
    char message[256] = "Hello Server";

    sendto(clientsocket,
           message,
           sizeof(message),
           0,
           (struct sockaddr *)&serveraddress,
           sizeof(serveraddress));

    // Receive reply from server
    int len = sizeof(serveraddress);

    recvfrom(clientsocket,
             serverresponse,
             sizeof(serverresponse),
             0,
             (struct sockaddr *)&serveraddress,
             &len);

    printf("The reply from the server is : %s\n", serverresponse);

    close(clientsocket);

    return 0;
}
