#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>

void *worker_thread(void *args);

struct dati{
    char messaggio[140];
    char utente[50];
    int id;
};
struct dati ultimo_msg;
pthread_mutex_t mutex;

int main(){
    pthread_t tid;
    pthread_attr_t tattr;
    ultimo_msg.id=0;
    ultimo_msg.messaggio[0]='\0';
    ultimo_msg.utente[0]='\0';
    int ssock,csock;
    struct sockaddr_in saddr;//server address

    pthread_mutex_init(&mutex,NULL);

    if(pthread_attr_init(&tattr)){perror("Errore init attributi thread.\n");exit(1);}
    if(pthread_attr_setdetachstate(&tattr,PTHREAD_CREATE_DETACHED)){perror("Errore setdetatchedstate.\n");exit(1);}
    
    ssock=socket(AF_INET,SOCK_STREAM,0);
    if(ssock==-1){perror("Errore creazione unnamed socket.\n");exit(EXIT_FAILURE);}
    
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9876);//htons converte short type(16bit) to network(big endian)
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);//htonl converte long type(32 bit) to network(big endian)
    if(bind(ssock,(struct sockaddr *)&saddr,sizeof(saddr))){perror("Errore bind()\n"); exit(EXIT_FAILURE);}

    listen(ssock,5);

    while(1){
        csock = accept(ssock,NULL,NULL);
        if(pthread_create(&tid,&tattr,worker_thread,(void *)csock)){perror("Errore creazione thread.\n");exit(1);}
    }

}

void *worker_thread(void *args){
    int sock=(int)args;
    char buffer[255];
    char utente[50];
    int len;

    while(1){
        len=read(sock,buffer,255);
        if(strncmp("read",buffer,4)==0){
            pthread_mutex_lock(&mutex);//controlloerrore
            snprintf(buffer,255,"%d %s: %s",ultimo_msg.id,ultimo_msg.utente,ultimo_msg.messaggio);
            pthread_mutex_unlock(&mutex);//controlloerrore
            write(sock,buffer,strlen(buffer)+1);
        }else if(strncmp("close",buffer,5)==0){
            close(sock);
            break;
        }else if(strncmp("send",buffer,4)==0){
            pthread_mutex_lock(&mutex);//ce
            ultimo_msg.id++;
            strcpy(ultimo_msg.utente,utente);
            strncpy(ultimo_msg.messaggio,&buffer[5],len-5);
            pthread_mutex_unlock(&mutex);//ce
            sprintf(buffer,"ok");
            write(sock,buffer,strlen(buffer)+1);
        }else if(strncmp("register",buffer,8)==0){
            strncpy(utente,&buffer[9],len-9);
            sprintf(buffer,"Welcome %s.",utente);
            write(sock,buffer,strlen(buffer)+1);
        }
    }


}