#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include <errno.h>
#include<unistd.h>
#include<stdlib.h>

int main()
{
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
	server_addr.sin_port = htons(5000);
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
			char* portMsg = "PORT 127,0,0,1,19,137"; //19*256+137

			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
			int value  = 1;
			setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value));
			struct sockaddr_in localaddr;
			bzero(&localaddr,sizeof(localaddr));
			localaddr.sin_family = AF_INET;
			localaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
			localaddr.sin_port = htons(6093);
			int bind_err = bind(sockfd, (struct sockaddr *)&localaddr, sizeof(localaddr));
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
			printf("RECEIVED 2: %s\n", filePP);
			close(sockfd);
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
