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
#define SERVER_DATA_PORT 5001

typedef struct user_state
{
	char *user;
	int client_num;
	int log ;  //check if user is looged in or not 
	int pass ; //check if the oassword is right?
	struct user_state *next; 
	char path[256];
}USR; 

USR* head= NULL;

void addNode(  int n)
{
	USR* node = (USR*) malloc(sizeof(USR)); 
	node->user = "";
	node->log = 0 ; 
	node->client_num = n;
	node-> pass = 0;
	node->next = head;
	head = node; 
	getcwd(node->path, 256);
}

USR* getNode(int n){
	USR* curr = head;   
	while(curr!=NULL)
	{	//printf("%duser updated with name%d\n", curr->client_num, n);
		if(curr->client_num==n){
			return curr;
	}
		curr = curr->next;
	}
	return NULL;
}

void user_log_in(char buff[], int n)
{
	char* us;
    us = malloc(strlen(buff)*sizeof(char)+1);
    strcpy(us, buff);
	USR* curr = head;   
	while(curr!=NULL)
	{	//printf("%duser updated with name%d\n", curr->client_num, n);
		if(curr->client_num==n){
			curr->log=1;
			curr->user= us;
			//printf("%suser updated with name\n", curr->user);
			break; 
	}
		curr = curr->next;
	}

}

int check_log_in(int n)
{
	int f=0;
	USR* curr = head; 
	while(curr!=NULL)
	{
		if(curr->client_num==n){
			if(curr->pass==1)
			{
				f=1;
			}	
			break; 
	}
		curr = curr->next;
	}
	return f;
}
void print_list()
{	
	USR* curr = head; 
	while(curr!=NULL)
	{
		curr= curr->next;
	}
}

int password_verified( char buff[])
{
	int f =0; 
	char* us;
    us = malloc(strlen(buff)*sizeof(char)+1);
	USR* curr = head; 
    strcpy(us, buff);
	printf("%s:us\n", us);
	while(curr!=NULL)
	{	
		//printf("%slisttt\n", curr->user);
		if(strcmp(curr->user,us)==0){
			curr->pass=1; 
			//printf("here password found\n");
			f=1;
			break; 
	}
		curr = curr->next;
	}
	return f;
}
void removeNode( int n)
{
	USR* curr= head;
	USR* prev = NULL; 
	while(curr!=NULL)
	{
		if(curr->client_num ==n)
		{
			if(prev==NULL)
			{
				head= curr->next;
			}
			else{
			prev->next= curr->next;
			}
			free(curr);
		break; }

		prev= curr; 
		curr= curr->next; 
	}
}

int main()
{
	USR* head = NULL;  //the lsit tp maintain state 
	FILE * file ;	
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
	char user_filepath[256];
	getcwd(user_filepath, 256);
	strcat(user_filepath, "/user.txt");

	char original_filepath[256];
	getcwd(original_filepath, 256);
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
	int* data_socks = (int*) malloc(3*sizeof(int));
	struct sockaddr_in serverDataAddr;
	bzero(&serverDataAddr,sizeof(serverDataAddr));
	serverDataAddr.sin_family = AF_INET;
	serverDataAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverDataAddr.sin_port = htons(SERVER_DATA_PORT);
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
				chdir(original_filepath);
				if(fd==server_sd)
				{
					int client_sd = accept(server_sd,0,0);
					printf("Client Connected fd = %d \n",client_sd);
					addNode(client_sd);
					FD_SET(client_sd,&full_fdset);
					send(client_sd, "220 Service ready for new user", strlen("220 Service ready for new user"), 0);
					
					if(client_sd>max_fd){
						max_fd = client_sd;
						data_socks = realloc(data_socks, sizeof(int)*max_fd);
					}
				}
				else
				{
					char buffer[256];
					bzero(buffer,sizeof(buffer));
					int bytes = recv(fd,buffer,sizeof(buffer),0);
					USR* curr_user = getNode(fd);
					if(curr_user!=NULL){
						chdir(curr_user->path);
					}
					if(bytes==0)   //client has closed the connection
					{
						printf("connection closed from client side \n");
						removeNode(fd);
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
						if(strncmp(buffer, "PORT", 4)==0){
							if(check_log_in(fd) == 0){
								send(fd, "530 Not logged in.", strlen("530 Not logged in."), 0);
								continue;
							}
							char* ret = "200 PORT command successful";
							int act_port[2];
							int act_ip[4];
							char ip[40];
							int port_dec;
							sscanf(buffer, "PORT %d,%d,%d,%d,%d,%d",&act_ip[0],&act_ip[1],&act_ip[2],&act_ip[3],&act_port[0],&act_port[1]);
							sprintf(ip, "%d.%d.%d.%d", act_ip[0], act_ip[1], act_ip[2],act_ip[3]);
							port_dec=act_port[0]*256+act_port[1];
							printf("ip: %s, port: %d\n", ip, port_dec);
							data_socks[fd] = socket(AF_INET, SOCK_STREAM, 0);
							printf("socket at: %d\n", data_socks[fd]);
							if(data_socks[fd] < 0){
								printf("socket err, errno: %d, sock:%d\n", errno, data_socks[fd]);
							}
							int value  = 1;
							setsockopt(data_socks[fd],SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value));
							struct sockaddr_in remoteaddr;
							bzero(&remoteaddr,sizeof(remoteaddr));
							remoteaddr.sin_family = AF_INET;
							remoteaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
							remoteaddr.sin_port = htons(port_dec); // get from port command above (client port to send data to)

							int bindErr = bind(data_socks[fd], (struct sockaddr *)&serverDataAddr, sizeof(serverDataAddr));
							if(bindErr<0){
								send(fd, "500 SERVER ERROR, PLEASE TRY AGAIN", strlen("500 SERVER ERROR, PLEASE TRY AGAIN"), 0);
								close(data_socks[fd]);
								continue;
							}
							int err = connect(data_socks[fd], (struct sockaddr *)&remoteaddr, sizeof(remoteaddr));
							if(err<0){
								send(fd, "500 SERVER ERROR, PLEASE TRY AGAIN", strlen("500 SERVER ERROR, PLEASE TRY AGAIN"), 0);
								close(data_socks[fd]);
								continue;
							}
							send(fd, ret, strlen(ret), 0); // send 200
							
						}else if(strncmp(buffer, "RETR",4)==0){
							if(check_log_in(fd) == 0){
								send(fd, "530 Not logged in.", strlen("530 Not logged in."), 0);
								continue;
							}
							char filepath[1024];
							sscanf(buffer, "RETR %s", filepath);
							FILE* fileobj = fopen(filepath, "r");
							if(!fileobj){
								char* notFound = "550 No such file or directory.";
								send(fd, notFound, strlen(notFound), 0); // send 404 not found
							}else{
								fseek(fileobj, 0, SEEK_SET);
								char* found = "150 File status okay; about to open data connection.";
								send(fd, found, strlen(found), 0); // send 150 file status okay
								//send file over data connection

								char send_buffer[256];
								memset(send_buffer,'\0',256);
								while(fgets(send_buffer,256,fileobj)!=NULL){
									send(data_socks[fd], send_buffer, 256, 0);
									memset(send_buffer,'\0',256);
								}
								send(fd, "226 Transfer completed.", strlen("226 Transfer completed."), 0);
							}
							fclose(fileobj);
							close(data_socks[fd]);
						}else if(strncmp(buffer, "STOR",4)==0){
							if(check_log_in(fd) == 0){
								send(fd, "530 Not logged in.", strlen("530 Not logged in."), 0);
								continue;
							}
							char* found = "150 File status okay; about to open data connection.";
							send(fd, found, strlen(found), 0); // send 150 file status okay
							char tempFileName[256];
							// Prints "Hello world!" on hello_world
							sprintf(tempFileName, "rand%d.tmp", fd);
							//receive file over data connection
							char fileBuffer[256];
							FILE* temp = fopen(tempFileName, "w");
							while(recv(data_socks[fd], fileBuffer, 256, 0) > 0){
								fwrite(fileBuffer, 1, strlen(fileBuffer), temp);
							}
							fclose(temp);
							char filepath[1024];
							sscanf(buffer, "RETR %s", filepath);
							rename(tempFileName, filepath);
							send(fd, "226 Transfer completed.", strlen("226 Transfer completed."), 0);
							close(data_socks[fd]);
						}else if(strncmp(buffer, "USER",4)==0){
							file  = fopen(user_filepath, "r");
							char temp[100]; 
							strncpy(temp, buffer+5, strlen(buffer) -5); 
							temp[strlen(buffer)- 5]= '\0';  //user name the client send
							char *ret; 
							char data[100];
							char * t =",";
							int f= 0;
							while(fgets(data, 200, file)){
								char* token1 = strtok(data, t);
								if(strcmp(token1, temp)==0){
										user_log_in( temp, fd); 
										f=1;
										ret ="331 Username OK, need password.";
										// send(fd, ret, strlen(ret), 0);
										break;
							}}
							if(f==0)
							{
								ret ="user not logged in \n";
							}
							if(send(fd, ret, strlen(ret), 0)<0)
							{
								printf("send error\n");
							}
							fclose(file);
						}else if(strncmp(buffer, "PASS",4)==0){
							file  = fopen(user_filepath, "r");
							if(file == NULL)
							{
								printf("fdsfds\n");
							}
							char temp[100]; 
							strncpy(temp, buffer+5, strlen(buffer) -5); 
							temp[strlen(buffer)- 5]= '\0';  //password
							char *ret; 
							char data[100];
							int f =0;
							char * t =",";
							// printf("here\n");
							while(fgets(data, 100, file)){
								char* token1 = strtok(data, t);
								char* token2 = strtok(NULL, t);
								token2[strlen(token2)-1]='\0';
								//printf("%s\t%s\t%s\n", token1, token2, temp);
								if(strcmp(token2, temp)==0){
										//printf("the password matches in the file \n");
										f = password_verified( token1); 
										ret ="User logged in, proceed.";
										break;
							}
							}
							if(f==0)
							{
								ret = "INVALID PASSWORD";
							}
							send(fd, ret, strlen(ret), 0);
							fclose(file);
						}else if(strncmp(buffer, "LIST", 4) == 0){
							if(check_log_in(fd) == 0){
								send(fd, "530 Not logged in.", strlen("530 Not logged in."), 0);
								continue;
							}
							char* found = "150 File status okay; about to open data connection.";
							send(fd, found, strlen(found), 0); // send 150 file status okay
							//receive file over data connection
							FILE *fpipe = popen("ls", "r");
							char send_buffer[256];
							memset(send_buffer,'\0',256);
							while(fgets(send_buffer,256,fpipe)!=NULL){
								send(data_socks[fd], send_buffer, 256, 0);
								memset(send_buffer,'\0',256);
							}
							pclose(fpipe);
							send(fd, "226 Transfer completed.", strlen("226 Transfer completed."), 0);
							close(data_socks[fd]);
						}else if(strncmp(buffer, "QUIT",4)==0){
							send(fd, "221 Service closing control connection", strlen("221 Service closing control connection"), 0);
							removeNode(fd);
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
						}else if(strncmp(buffer, "CWD",3)==0){
							if(check_log_in(fd) == 0){
								send(fd, "530 Not logged in.", strlen("530 Not logged in."), 0);
								continue;
							}
							char filepath[1024];
							bzero(filepath, 256);
							sscanf(buffer, "CWD %s", filepath);
							int chdir_error = chdir(filepath);
							if(chdir_error < 0){
								char* fail = "550 No such file or directory";
								send(fd, fail, strlen(fail), 0);
							}else{
								USR* user = getNode(fd);
								bzero(user->path, 256);
								getcwd(user->path, 256);
								char success[256] = "200 directory changed to ";
								strcat(success, user->path);
								send(fd, success, strlen(success), 0);
							}
						}else if(strncmp(buffer, "PWD",3)==0){
							if(check_log_in(fd) == 0){
								send(fd, "530 Not logged in.", strlen("530 Not logged in."), 0);
								continue;
							}
							USR* user = getNode(fd);
							char success[256] = "257 ";
							strcat(success, user->path);
							send(fd, success, strlen(success), 0);
						}else{
							if(check_log_in(fd) == 0){
								send(fd, "530 Not logged in.", strlen("530 Not logged in."), 0);
								continue;
							}
							char* ret = "202 Command not implemented";
							send(fd, ret, strlen(ret), 0);
						}

					}
				}
			}
		}

	}

	//close
	free(data_socks);
	close(server_sd);
	return 0;
}
