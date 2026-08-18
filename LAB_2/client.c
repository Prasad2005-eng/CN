#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

int main()
{
//create Socket
int clientsocket,a,b,sum;
//char serverresponse[256];
struct sockaddr_in serveraddress;

clientsocket=socket(AF_INET,SOCK_STREAM,0);// create socket
//SOCKET function returns the integer
if(clientsocket<0)
printf("socket is not created\n");
else 
printf("socket is created succesfully\n");

serveraddress.sin_family=AF_INET;
serveraddress.sin_port=htons(9000);
//serveraddress.sin_addr.s_addr= htons("192.168.56.1");
serveraddress.sin_addr.s_addr= INADDR_ANY;

int constatus=connect(clientsocket,(struct sockaddr *) &serveraddress, sizeof(serveraddress));
 
 if(constatus==-1)
 printf("there was an error in the connection\n");
 else 
 printf("connection is estableshed");
 

 char serverresponse[256];
 int len=sizeof(serverresponse);
 recv(clientsocket,&serverresponse,len,0);
 
 printf("The reply from the server is:%s\n",serverresponse);
 
 close(clientsocket);
 
 return 0;
 
 }
