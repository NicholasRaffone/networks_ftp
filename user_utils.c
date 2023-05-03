typedef struct user_state
{
	char *user;
	int client_num;
	int log ;  //check if user is looged in or not 
	int pass ; //check if the oassword is right?
	struct user_state *next; 
}USR; 
void addNode(  int n)
{
	// char* us;
    // us = malloc(strlen(buff)*sizeof(char)+1);
    // strcpy(us, buff);
	USR* node = (USR*) malloc(sizeof(USR)); 
	// free(us);
	node->user = "";
	node->log = 0 ; 
	node->client_num = n;
	node-> pass = 0;
	node->next = head;
	head = node; 
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
		//printf("%d\tname:%s\n", curr->client_num, curr->user);
		curr= curr->next;
	}
}

int password_verified( char buff[])
{
	int f =0; 
	char* us;
    us = malloc(strlen(buff)*sizeof(char)+1);
    strcpy(us, buff);
	USR* curr = head; 
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
