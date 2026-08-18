#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>

int main()
{
    int serversocket, bindstatus;

    char servermessage[256] = "Hello Client and we are connected now";
    char clientmessage[256];

    struct sockaddr_in serveraddress, clientaddress;

    int len = sizeof(clientaddress);

    serversocket = socket(AF_INET, SOCK_DGRAM, 0);

    if(serversocket < 0)
        printf("Socket failed\n");
    else
        printf("Socket created successfully\n");

    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(9000);
    serveraddress.sin_addr.s_addr = INADDR_ANY;

    bindstatus = bind(serversocket,
                      (struct sockaddr *)&serveraddress,
                      sizeof(serveraddress));

    if(bindstatus < 0)
        printf("Binding failed\n");
    else
        printf("Binding is successful\n");

    printf("Waiting for client...\n");

    // Receive message from client
    recvfrom(serversocket,
             clientmessage,
             sizeof(clientmessage),
             0,
             (struct sockaddr *)&clientaddress,
             &len);

    printf("Message from Client : %s\n", clientmessage);

    // Send reply
    sendto(serversocket,
           servermessage,
           sizeof(servermessage),
           0,
           (struct sockaddr *)&clientaddress,
           len);

    close(serversocket);

    return 0;
}
