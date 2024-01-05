#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

char name[NAME_SIZE];
char msg[BUF_SIZE];
int first_notice=0;

void * send_msg(void *arg){
    time_t t;
    struct tm *tm_info;

    time(&t);
    tm_info = localtime(&t);

    int sock=*((int *)arg);
    char name_msg[NAME_SIZE+BUF_SIZE];
    char message[NAME_SIZE+BUF_SIZE+10];
    
    while(1){
        fgets(msg,BUF_SIZE,stdin);
        if(!strcmp(msg,"q\n")|!strcmp(msg,"Q\n")){
            close(sock);
            exit(0);
        }
        sprintf(message,"%d:%d %s %s",tm_info->tm_hour,tm_info->tm_min,name,msg);
        write(sock,message,strlen(message));
    }
    return NULL;
}

void * rcv_msg(void *arg){
    int sock=*((int *)arg);
    char name_msg[NAME_SIZE+BUF_SIZE+10];
    int str_len;
    while(1){
        str_len=read(sock,name_msg,NAME_SIZE+BUF_SIZE);
        if(str_len==-1){
            return (void *)-1;
        }
        name_msg[str_len]=0;
        fputs(name_msg,stdout);
    }
    return NULL;
}

void error_handling(char *msg){
    fputs(msg,stderr);
    fputc('\n',stderr);
    exit(1);
}

int main(int argc, char *argv[]){
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t send_thread,rcv_thread;
    void *thread_return;
    
    sprintf(name,"[%s]",argv[3]);

    sock=socket(PF_INET,SOCK_STREAM,0);

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_addr.sin_port=htons(atoi(argv[2]));

    if(connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr))==-1){
        error_handling("connect error()");
    }
    printf("if you press 'q' or 'Q', you can exit\n");

    pthread_create(&send_thread,NULL,send_msg,(void *)&sock);
    pthread_create(&rcv_thread,NULL,rcv_msg,(void *)&sock);
    pthread_join(send_thread,&thread_return);
    pthread_join(rcv_thread,&thread_return);
    close(sock);
    return 0;
}