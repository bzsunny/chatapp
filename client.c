#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <semaphore.h>
#define SIZE 128
sem_t sem1,sem2;
int sockfd;
enum {run,stop};
int status = run;
void* readThread(void*);
void* writeThread(void*);
int main(int argc, char const *argv[])
{
	
	sem_init(&sem1,0,1);
	sem_init(&sem2,0,0);
	struct sockaddr_in address;
	int len,result;
	pthread_t tid_r,tid_w;
	sockfd = socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK,0);

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("0.0.0.0");
	address.sin_port = htons(9332);
	len = sizeof(address);
	result = connect(sockfd,(struct sockaddr*)&address,len);
	pthread_create(&tid_r,NULL,readThread,NULL);
	pthread_create(&tid_w,NULL,writeThread,NULL);
	pthread_join(tid_r,NULL);
	pthread_join(tid_w,NULL);
	close(sockfd);
	return 0;
}

void *writeThread(void *data){
	char send_buf[SIZE];
	while(1){
		read(0,send_buf,SIZE);

		write(sockfd,send_buf,strlen(send_buf)+1);
		if(strcmp(send_buf,"EXIT")==0 || strcmp(send_buf,"exit")==0 ){
			status = stop;
			break;
		}
	}
}

void *readThread(void* data){
	char rec_buf[SIZE];
	while(1){
	if(status == stop){
		break;
	}
	if(read(sockfd,rec_buf,SIZE)>0)	
	{
		printf("read from server: %s\n",rec_buf);
	}
	}
}
