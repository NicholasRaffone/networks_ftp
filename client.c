#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include <errno.h>
#include<unistd.h>
#include<stdlib.h>
#define SERVER_DATA_PORT 5001
#define SERVER_CONTROL_PORT 5000

int main(int argc, char** argv)
{
	int port_offset = 1;
	int CLIENT_CONTROL_PORT = atoi(argv[1]);
	//socket
	int server_sd = socket(AF_INET,SOCK_STREAM,0);
	if(server_sd<0)
	{
		perror("socket:");
		exit(-1);
	}
	//setsock
	int value  = 1;
	setsockopt(server_sd,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value)); //&(int){1},sizeof(int)
	struct sockaddr_in server_addr;
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_CONTROL_PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //INADDR_ANY, INADDR_LOOP

	//connect
    if(connect(server_sd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
    {
        perror("connect");
        exit(-1);
    }
	
	//accept
	char buffer[256];
	char retBuffer[256];

	while(1)
	{
       fgets(buffer,sizeof(buffer),stdin);
       buffer[strcspn(buffer, "\n")] = 0;  //remove trailing newline char from buffer, fgets does not remove it
       if(strcmp(buffer,"bye")==0)
        {
        	printf("closing the connection to server \n");
        	close(server_sd);
            break;
        }

		// HANDLE DIFFERENT SHIT

		if(strncmp(buffer, "RETR", 4) == 0){
			// retr flow
			// char* portMsg = "PORT 127,0,0,1,19,137"; //19*256+137
			char portMsg[40];
			int client_data_port = CLIENT_CONTROL_PORT+port_offset;
			sprintf(portMsg, "PORT 127,0,0,1,%d,%d", (int)client_data_port/256, client_data_port%256);
			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
			int value  = 1;
			setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value));
			struct sockaddr_in clientDataAddr;
			bzero(&clientDataAddr,sizeof(clientDataAddr));
			clientDataAddr.sin_family = AF_INET;
			clientDataAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
			clientDataAddr.sin_port = htons(client_data_port); // N+1 (data to send from)

			// for uploading data, establish port command in same way, but bind to local, send data to remote

			int bind_err = bind(sockfd, (struct sockaddr *)&clientDataAddr, sizeof(clientDataAddr));
			if(bind_err < 0){
				printf("bind err, errno:%d\n", errno);
				exit(1);
			}
			int listen_err = listen(sockfd,5);
			if(listen_err!=0){
				printf("listen err\n");
				close(sockfd);
				exit(1);
			}
			
			send(server_sd, portMsg,strlen(portMsg),0); // send port x:y
			int accept_val = accept(sockfd, 0,0);

			recv(server_sd, retBuffer, 256, 0);
			printf("RECEIVED: %s\n", retBuffer); // receive 200 port success
			
			send(server_sd, buffer, 256, 0); // send retr filename
			
			recv(server_sd, retBuffer, 256, 0);
			printf("RECEIVED: %s\n", retBuffer); // receive 150 file success
			char filePP[256];
			// receive file
			recv(accept_val, filePP, 256, 0);
			printf("RECEIVED 2: %s\n", filePP); // file data
			recv(accept_val, filePP, 256, 0);
			printf("RECEIVED 2: %s\n", filePP); // 226 file transfer completed
			close(sockfd);
			port_offset++;
		}
		else if(strncmp(buffer, "STOR", 4) == 0){
			char portMsg[40];
			int client_data_port = CLIENT_CONTROL_PORT+1;
			sprintf(portMsg, "PORT 127,0,0,1,%d,%d", (int)client_data_port/256, client_data_port%256);
			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
			int value  = 1;
			setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value));
			struct sockaddr_in clientDataAddr;
			bzero(&clientDataAddr,sizeof(clientDataAddr));
			clientDataAddr.sin_family = AF_INET;
			clientDataAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
			clientDataAddr.sin_port = htons(client_data_port); // N+1 (data to send from)

			// for uploading data, establish port command in same way, but bind to local, send data to remote

			int bind_err = bind(sockfd, (struct sockaddr *)&clientDataAddr, sizeof(clientDataAddr));
			if(bind_err < 0){
				printf("bind err, errno:%d\n", errno);
				exit(1);
			}
			int listen_err = listen(sockfd,5);
			if(listen_err!=0){
				printf("listen err\n");
				close(sockfd);
				exit(1);
			}
			
			send(server_sd, portMsg,strlen(portMsg),0); // send port x:y
			int accept_val = accept(sockfd, 0,0);

			recv(server_sd, retBuffer, 256, 0);
			printf("RECEIVED: %s\n", retBuffer); // receive 200 port success
			
			send(server_sd, buffer, 256, 0); // send retr filename
			
			recv(server_sd, retBuffer, 256, 0);
			printf("RECEIVED: %s\n", retBuffer); // receive 150 file success
			// send file
			send(accept_val, "HELLO", strlen("HELLO"), 0); // send data on data connection

			char filePP[256];
			recv(accept_val, filePP, 256, 0);
			printf("RECEIVED 2: %s\n", filePP); // 226 file transfer completed
			close(sockfd);
			port_offset++;
		}else{
			printf("COUDL NOT FIND\n");
			int err = send(server_sd,buffer,strlen(buffer),0);
			if(err<0)
			{
				perror("send");
				exit(-1);
			}
			bzero(buffer,sizeof(buffer));
			recv(server_sd, retBuffer, 256, 0);
			printf("RECEIVED: %s\n", retBuffer);
		}
	}

	return 0;
}
