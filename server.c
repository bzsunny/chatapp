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
	int sock_fd;
	unsigned long int c_ip;
	unsigned short int c_port;
	char status[10];
};

sem_t sem1,sem2;

typedef void*(funptr)(void*);
struct table socket_table[SIZE];
int cur_pos=0;


void check_client(int);
void* handleThread(void*);
void client_accept(int*);
void table_update(unsigned long int, unsigned short int,int fd);
int table_check(unsigned long int, unsigned short int,int fd);

 int main(int argc, char const *argv[])
 {
 	sem_init(&sem1,0,1);
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

void table_update( unsigned long int ip, unsigned short int port ,int client_sockfd){
			printf("Updating table\n");
			sem_wait(&sem1);
			if(table_check(ip,port,client_sockfd)==0){
			socket_table[cur_pos].sock_fd = client_sockfd;
			socket_table[cur_pos].c_ip = ip;
			socket_table[cur_pos].c_port = port;
			strcpy(socket_table[cur_pos].status,"open");
			cur_pos++;
			sem_post(&sem1);
		}
}

void* handleThread(void* sock_fd){
		int fd = (int*)sock_fd;
		char ch[SIZE];
		int i=0;
		while(read(fd,ch,SIZE)>0){	
			printf(" In thread\n");
			for(i=0;i<=cur_pos;i++)
				{
						sem_wait(&sem1);
						if(strcmp(socket_table[i].status,"open")==0){
							write(socket_table[i].sock_fd,ch,strlen(ch)+1);
						}
						sem_post(&sem1);
				} 
			
 	}
}

int table_check(unsigned long int ip, unsigned short int port,int client_sockfd){
		int i=0;
		for(i=0;i<=cur_pos;i++){
			if(client_sockfd==socket_table[i].sock_fd){
				if(ip == socket_table[i].c_ip){
					if(port == socket_table[i].c_port){
						return 1;
					}
				}
			}
		}
		return 0;
}
