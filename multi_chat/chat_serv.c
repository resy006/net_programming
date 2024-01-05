#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

char chat_log_file[] = "file1.txt";  // 채팅 로그를 저장할 파일명

void Record_file(const char *file_path, const char *content) {
    FILE *file = fopen(file_path, "a");
    if (file == NULL) {
        perror("fopen");
        exit(1);
    }

    fputs(content, file);
    fclose(file);
}

void send_msg(char *msg,int len){
    int i;
    pthread_mutex_lock(&mutx);
    for(i=0;i<clnt_cnt;i++){
        write(clnt_socks[i],msg,len);
    }
    pthread_mutex_unlock(&mutx);
}



void *handle_clnt(void *arg){
    int clnt_sock=*((int *)arg);
    int str_len=0;
    char msg[BUF_SIZE];

    while((str_len=read(clnt_sock,msg,sizeof(msg)))!=0){
        send_msg(msg,str_len); // send message
        Record_file(chat_log_file, msg);
    }

    pthread_mutex_lock(&mutx);
    for(int i=0;i<clnt_cnt;i++){
        if(clnt_sock==clnt_socks[i]){ // same client socket deleate 
            while(i++<clnt_cnt-1){
                clnt_socks[i]=clnt_socks[i+1];
            }
            break;
        }
    }
    clnt_cnt --;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;
} 

void error_handling(char *msg){
    fputs(msg,stderr);
    fputc('\n',stderr);
    exit(1);
}

int main(int argc,char *argv[]){
    int serv_sock,clnt_sock;
    struct sockaddr_in serv_adr,clnt_adr;
    int clnt_adr_sz;
    pthread_t t_id;

    pthread_mutex_init(&mutx,NULL);
    serv_sock=socket(PF_INET,SOCK_STREAM,0);
    memset(&serv_adr,0,sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sock,(struct sockaddr *)&serv_adr,sizeof(serv_adr))==-1){
        error_handling("bind error()");
    }
    if(listen(serv_sock,5)==-1){
        error_handling("listen error()");
    }

    while(1){
        clnt_adr_sz=sizeof(clnt_adr);
        clnt_sock=accept(serv_sock,(struct sockaddr *)&clnt_adr,&clnt_adr_sz);

        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++]=clnt_sock;
        pthread_mutex_unlock(&mutx);

        pthread_create(&t_id,NULL,handle_clnt,(void *)&clnt_sock);
        pthread_detach(t_id);
        printf("connected client IP : %s \n",inet_ntoa(clnt_adr.sin_addr));
        
    }
    close(clnt_sock);
    return 0;
}
