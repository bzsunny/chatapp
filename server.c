#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <semaphore.h>

#define SIZE 128
#define MAX 5

struct table{
	sem_t sem;
	int sock_fd;
	unsigned long int c_ip;
	unsigned short int c_port;
	char status[10];
};

sem_t cur_sem,sem2;

typedef void*(funptr)(void*);
struct table socket_table[SIZE];
int cur_pos=0;


void check_client(int);
void* handleThread(void*);
void client_accept(int*);
void table_update(unsigned long int, unsigned short int,int fd);
int table_check(unsigned long int, unsigned short int,int fd);
void semaphore_init(void);

 int main(int argc, char const *argv[])
 {
 	sem_init(&cur_sem,0,1);
 	sem_init(&sem2,0,0);
 	int server_sockfd,client_sockfd;
 	char buffer[SIZE];
 	pthread_t tid[MAX];
 	pid_t pid;
 	struct sockaddr_in server_addr,client_addr;
 	int server_len,client_len;
 	server_sockfd = socket(AF_INET,SOCK_STREAM,0);
 	server_addr.sin_family = AF_INET;
 	server_addr.sin_addr.s_addr= htonl(INADDR_ANY);
 	server_addr.sin_port = htons(9332);
 	server_len = sizeof(server_addr);
 	semaphore_init();
 	bind(server_sockfd,(struct sockaddr*)&server_addr,server_len);
 	listen(server_sockfd, 5);
 	printf("server waiting\n");
 	client_len = sizeof(client_addr);
 	while((client_sockfd = accept(server_sockfd,(struct sockaddr*)&client_addr,&client_len))!=-1){
 		
 		table_update( ntohl(client_addr.sin_addr.s_addr),ntohs(client_addr.sin_port),client_sockfd);
 		
 		if((pthread_create(&tid,NULL,handleThread,(void*)client_sockfd)==0)){
 			printf("Accepted %d",client_sockfd);
 		}	
 	}
 	return 0;
 }


void semaphore_init(){
	int i;
	for(i=0;i<SIZE;i++)
	{
		sem_init(&socket_table[i].sem,0,1);
	}
	printf("done with init of semaphores\n");
}

void table_update( unsigned long int ip, unsigned short int port ,int client_sockfd){
			printf("Updating table\n");
			
			if(table_check(ip,port,client_sockfd)==0){
			sem_wait(&cur_sem);
			sem_wait(&socket_table[cur_pos].sem);
			socket_table[cur_pos].sock_fd = client_sockfd;
			socket_table[cur_pos].c_ip = ip;
			socket_table[cur_pos].c_port = port;
			strcpy(socket_table[cur_pos].status,"open");
			sem_post(&socket_table[cur_pos].sem);
			cur_pos++;
			sem_post(&cur_sem);
			printf("table updated\n");
		}
		else
		{
			printf("possible dublication in table\n");
		}
}

void* handleThread(void* sock_fd){
		int fd = (int*)sock_fd;
		char ch[SIZE];
		int i=0;
		printf(" In thread\n");
		while(read(fd,ch,SIZE)>0){	
			printf("read from: %d thread\n",fd);
			for(i=0;i<=cur_pos;i++)
				{
						sem_wait(&socket_table[i].sem);
						if(strcmp(socket_table[i].status,"open")==0 ){
							write(socket_table[i].sock_fd,ch,sizeof(ch));
						}
						sem_post(&socket_table[i].sem);
				} 
			
 	}
}

int table_check(unsigned long int ip, unsigned short int port,int client_sockfd){
		int i=0;
		for(i=0;i<=cur_pos;i++){
			sem_wait(&socket_table[i].sem);
			if(client_sockfd==socket_table[i].sock_fd){
				if(ip == socket_table[i].c_ip){
					if(port == socket_table[i].c_port){
						sem_post(&socket_table[i].sem);
						return 1;
					}
				}
			}
		   sem_post(&socket_table[i].sem);
		}
		return 0;
}
