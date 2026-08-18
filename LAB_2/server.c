#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

int main()
{
int serversocket,client_socket,bindstatus,a,b,sum=0;
char servermessage[256]="Hello Client and we are connected now";
struct sockaddr_in serveraddress;


serversocket=socket(AF_INET,SOCK_STREAM,0);
if(serversocket<0)
printf("socket failed");
serveraddress.sin_family=AF_INET;
serveraddress.sin_port=htons(9000);
//serveraddress.sin_addr.s_addr= htons("192.168.56.1");
serveraddress.sin_addr.s_addr= INADDR_ANY;

bindstatus=bind(serversocket,(struct sockaddr *)&serveraddress, sizeof(serveraddress));
if(bindstatus<0)
printf("binding failed\n");
else
printf("binding is successful\n");

listen(serversocket,3);
printf("Send reply to the client\n");

//clientsocket=accept(serversocket,( struct sockaddr *)&clientsocket, sizeof(clientsocket));
client_socket=accept(serversocket,NULL,NULL);
if(client_socket<0)
printf("connection is rejected by server\n");
else
printf("connection is accepted by server\n");
printf("\n");

send(client_socket,servermessage,sizeof(servermessage),0);

close(serversocket);
return 0;

}

