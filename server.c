//============================================================================
// Name         : Chat Server using Select()
// Description  : This Program will receive messages from several clients using
//                select() system class
//============================================================================
#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/select.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#define CONTROL_PORT 5000
#define DATA_PORT 5001

int main()
{
	//socket
	int server_sd = socket(AF_INET,SOCK_STREAM,0);
	printf("Server fd = %d \n",server_sd);
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
	server_addr.sin_port = htons(CONTROL_PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //INADDR_ANY, INADDR_LOOP

	//bind
	if(bind(server_sd, (struct sockaddr*)&server_addr,sizeof(server_addr))<0)
	{
		perror("bind failed");
		exit(-1);
	}
	//listen
	if(listen(server_sd,5)<0)
	{
		perror("listen failed");
		close(server_sd);
		exit(-1);
	}
	
	fd_set full_fdset;
	fd_set read_fdset;
	FD_ZERO(&full_fdset);

	int max_fd = server_sd;

	FD_SET(server_sd,&full_fdset);

	printf("Server is listening...\n");
	int sockfd_two;
	while(1)
	{
		printf("max fd = %d \n",max_fd);
		read_fdset = full_fdset;

		if(select(max_fd+1,&read_fdset,NULL,NULL,NULL)<0)
		{
			perror("select");
			exit (-1);
		}

		for(int fd = 3 ; fd<=max_fd; fd++)
		{
			if(FD_ISSET(fd,&read_fdset))
			{
				if(fd==server_sd)
				{
					int client_sd = accept(server_sd,0,0);
					printf("Client Connected fd = %d \n",client_sd);
					FD_SET(client_sd,&full_fdset);
					
					if(client_sd>max_fd)	
						max_fd = client_sd;
				}
				else
				{
					char buffer[256];
					bzero(buffer,sizeof(buffer));
					int bytes = recv(fd,buffer,sizeof(buffer),0);
					if(bytes==0)   //client has closed the connection
					{
						printf("connection closed from client side \n");
						close(fd);
						FD_CLR(fd,&full_fdset);
						if(fd==max_fd)
						{
							for(int i=max_fd; i>=3; i--)
								if(FD_ISSET(i,&full_fdset))
								{
									max_fd =  i;
									break;
								}
						}
					}else{

						// check here for commands: PORT, RETR, ETC.

						/*
							RETR FLOW:
							1. SERVER GETS `PORT XYZ` ON CONTROL CONNECTION:
								CLIENT OPENS DATA CONNECTION ON CLIENT PORT N+1 TO SEND TO SERVER PORT 21
							2. SERVER SENDS BACK 200 PORT COMMAND SUCCESSFUL ON CONTROL CONNECTION
							3. SERVER GETS `RETR FILE.TXT` ON CONTROL CONNECTION:
								SERVER CHECKS IF THE FILE EXISTS, IF EXISTS SERVER SENDS 150 FILE STATUS OKAY, OPENING
							4. SERVER OPEN PORT 21 AND GET READY TO SEND TO XYZ
							5. SERVER SENDS FILE FROM SERVER PORT 21 TO CLIENT PORT N+1
							6. SERVER CLOSES PORT AFTER SEND
							7. CLIENT CLOSES PORT AFTER RECEIVE
						*/
						if(strncmp(buffer, "PORT", 4)==0){
							printf("port command\n");
							char* ret = "200 PORT command successful";

							sockfd_two = socket(AF_INET, SOCK_STREAM, 0);
							int value  = 1;
							setsockopt(sockfd_two,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value));
							struct sockaddr_in remoteaddr;
							bzero(&remoteaddr,sizeof(remoteaddr));
							remoteaddr.sin_family = AF_INET;
							remoteaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
							remoteaddr.sin_port = htons(6093);
							int err = connect(sockfd_two, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr));
							if(err!=0){
								printf("connect err, %d\n", errno);
							}
							send(fd, ret, strlen(ret), 0); // send 200
							
						}else if(strncmp(buffer, "RETR",4)==0){
							printf("received: %s, fd:%d, sockval:%d \n",buffer, fd, sockfd_two);

							char* found = "150 File status okay; about to open data connection.";
							send(fd, found, strlen(found), 0); // send 150 file status okay
							//send file over data connection
							send(sockfd_two, "2.txtEEE", strlen("2.txtEEE"), 0);
							close(sockfd_two);
						}else{
							printf("INVALID COMMAND: %s\n", buffer);
							char* ret = "COMMAND NOT FOUND";
							send(fd, ret, strlen(ret), 0);
						}

					}
				}
			}
		}

	}

	//close
	close(server_sd);
	return 0;
}
