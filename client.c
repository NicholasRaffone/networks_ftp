#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include <errno.h>
#include<unistd.h>
#include<stdlib.h>
#include <time.h>
#define SERVER_DATA_PORT 5001
#define SERVER_CONTROL_PORT 5000

int connectSocket(char* portMsg, int dataportnum){
	sprintf(portMsg, "PORT 127,0,0,1,%d,%d", (int)dataportnum/256, dataportnum%256);
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int value  = 1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value));
	struct sockaddr_in clientDataAddr;
	bzero(&clientDataAddr,sizeof(clientDataAddr));
	clientDataAddr.sin_family = AF_INET;
	clientDataAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	clientDataAddr.sin_port = htons(dataportnum); // N+1 (data to send from)

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
	return sockfd;
}

int main(int argc, char** argv)
{
	int port_offset = 1;
	
	char user[1024];
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

	struct sockaddr_in local_addr;
	bzero(&local_addr,sizeof(local_addr));
	local_addr.sin_family = AF_INET;

    srand(time(NULL));
    int CLIENT_CONTROL_PORT = (rand() % (65353 - 1025 + 1)) + 1025;
	local_addr.sin_port = htons(CLIENT_CONTROL_PORT);
	local_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //INADDR_ANY, INADDR_LOOP

	bind(server_sd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	
	//connect
    if(connect(server_sd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
    {
        perror("connect");
        exit(-1);
    }
	
	printf("port %d\n", CLIENT_CONTROL_PORT);
	
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
			char portMsg[40];
			int sockfd = connectSocket(portMsg, CLIENT_CONTROL_PORT+port_offset);
			
			send(server_sd, portMsg,strlen(portMsg),0); // send port x:y
			bzero(retBuffer, 256);
			recv(server_sd, retBuffer, 256, 0);
			printf("%s\n", retBuffer); // receive 200 port success

			if(strncmp(retBuffer, "200", 3) == 0){
				printf("ACCEPTD\n");
				int accept_val = accept(sockfd, 0,0);
				send(server_sd, buffer, 256, 0); // send retr filename

				bzero(retBuffer, 256);
				recv(server_sd, retBuffer, 256, 0);
				printf("RECEIVED: %s\n", retBuffer); // receive 150 file success or 404

				if(strncmp("150", retBuffer, 3) == 0){
					// file is found
					printf("file found\n");
					char filePP[256];
					// receive file
					FILE* temp = fopen("test.txt", "w");
					while(recv(accept_val, filePP, 256, 0) > 0){
						fwrite(filePP, 1, strlen(filePP), temp);
					}
					fclose(temp);
					bzero(retBuffer, 256);
					recv(server_sd, retBuffer, 256, 0);
					printf("RECEIVED 2: %s\n", retBuffer); // 226 file transfer completed
				}
			}
			port_offset++;
			close(sockfd);
			printf("exiting retr\n");
		}else if(strncmp(buffer, "STOR", 4) == 0){
			char filepath[1024];
			sscanf(buffer, "STOR %s", filepath);
			FILE* fileobj = fopen(filepath, "r");
			if(!fileobj){
				printf("File not found\n");
			}else{
				fseek(fileobj, 0, SEEK_SET);
				char portMsg[40];
				int sockfd = connectSocket(portMsg, CLIENT_CONTROL_PORT+port_offset);
				
				send(server_sd, portMsg,strlen(portMsg),0); // send port x:y
				
				bzero(retBuffer, 256);
				recv(server_sd, retBuffer, 256, 0);
				printf("RECEIVED: %s\n", retBuffer); // receive 200 port success
				if(strncmp("200", retBuffer, 3) == 0){

					int accept_val = accept(sockfd, 0,0);

					send(server_sd, buffer, 256, 0); // send STOR filename

					bzero(retBuffer, 256);
					recv(server_sd, retBuffer, 256, 0);
					printf("RECEIVED: %s\n", retBuffer); // receive 150 file success

					//send file over data connection
					char send_buffer[256];
					memset(send_buffer,'\0',256);
					while(fgets(send_buffer,256,fileobj)!=NULL){
						send(accept_val, send_buffer, 256, 0);
						memset(send_buffer,'\0',256);
					}
					printf("done sending\n");
					close(accept_val);
					close(sockfd);

					bzero(retBuffer, 256);
					recv(server_sd, retBuffer, 256, 0);
					printf("RECEIVED 2: %s\n", retBuffer); // 226 file transfer completed
				}else{
					close(sockfd);
				}
				fclose(fileobj);
				port_offset++;
			}
		}else if(strncmp(buffer, "USER", 4)==0){

			send(server_sd, buffer,strlen(buffer),0);
			// printf("user\n");
			bzero(retBuffer,sizeof(retBuffer));
			if(recv(server_sd, retBuffer, 256, 0)<0)
			{
				perror("send");
				exit(-1);
			}
			// printf("here");
			printf("%s\n", retBuffer);	
		}else if(strncmp(buffer, "PASS", 4)==0){
			// printf("here");
			printf("buffer is: %s\n", buffer);
			send(server_sd, buffer,strlen(buffer),0);
			bzero(retBuffer,sizeof(retBuffer));
			if(recv(server_sd, retBuffer, 256, 0)<0)
			{
				perror("send");
				exit(-1);
			}
			printf("%s\n", retBuffer);
		}else if(strncmp(buffer, "CWD",3)==0){
			char portMsg[40];
			int sockfd = connectSocket(portMsg, CLIENT_CONTROL_PORT+port_offset);
			
			send(server_sd, portMsg,strlen(portMsg),0); // send port x:y
			bzero(retBuffer, 256);
			recv(server_sd, retBuffer, 256, 0);
			printf("%s\n", retBuffer); // receive 200 port success

			if(strncmp(retBuffer, "200", 3) == 0){
				int accept_val = accept(sockfd, 0,0);
				send(server_sd, buffer, 256, 0); // send retr filename

				bzero(retBuffer, 256);
				recv(server_sd, retBuffer, 256, 0);
				printf("%s\n", retBuffer); // receive 150 file success or 404

				if(strncmp("150", retBuffer, 3) == 0){
					// file is found
					char filePP[256];
					// receive file
					while(recv(accept_val, filePP, 256, 0) > 0){
						printf("%s", filePP);
					}
					bzero(retBuffer, 256);
					recv(server_sd, retBuffer, 256, 0);
					printf("%s\n", retBuffer); // 226 file transfer completed
				}
			}
			port_offset++;
			close(sockfd);
		}else{
			printf("INVALID COMMAND\n");
		}
	}

	return 0;
}
