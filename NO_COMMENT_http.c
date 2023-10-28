#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>
#define SMALL_SIZE 30
#define BIG_SIZE 2048

void *request(void *sock);                    
void error_message(char *arg);               
void response(char *type, int sock, int i);    
char *ret_type(int i);                        

int main(int argc, char *argv[])                
{
    int serv_sock, clnt_sock;                  
    struct sockaddr_in serv_addr, clnt_addr;   
    int clnt_addr_size;                       
    pthread_t gg;                            
    if(argc != 2)                              
    {
        printf("please port : %s\n", argv[0]); 
        exit(1);                              
    }

    //serv_addr 초기화
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);        
    memset(&serv_addr, 0, sizeof(serv_addr));           
    serv_addr.sin_family = AF_INET;                     
    serv_addr.sin_port = htons(atoi(argv[1]));          
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);      
    
    if(bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)      
        error_message("bind error");
    
    if(listen(serv_sock, 10) == -1)                    
        error_message("listen error");
    
    while(1)
    {
        clnt_addr_size = sizeof(clnt_addr);               
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size); 
        if (clnt_sock == -1)
            error_message("accept error");
        pthread_create(&gg, NULL, request, &clnt_sock);   
        pthread_detach(gg);                              
    }
    close(serv_sock);
    return 0;
}
void *request(void *sock)               
{
    char title[SMALL_SIZE]={0,};      
    char type[15] = {0,};                
    int clnt_sock = *(int *)sock;        

    read(clnt_sock, title, 30);          
    if(strstr(title, "html") != NULL)    
    {
        strcpy(type, ret_type(1));     
        response(type, clnt_sock, 1);     
    }
    else if(strstr(title, "jpg") != NULL)  
    {
        strcpy(type, ret_type(1));       
        response(type, clnt_sock, 2);     
    }
    else
        strcpy(type, ret_type(1));        
        response(type, clnt_sock, 3);     
}
char* ret_type(int i)
{
    if (i == 1)
        return "text/html";            
}
void response(char *type, int sock, int i)
{
    char buf[BIG_SIZE] = {0,};                           
    int fp;                                           
    if (i == 1)                                         
        fp = open("index.html", O_RDONLY);             
    else if (i == 2)                                    
        fp = open("index2.html", O_RDONLY);             
    else if (i == 3)                                     
        {
            char Status[] = "HTTP/1.1 404 Not found\n"; 
            write(sock, Status, strlen(Status));
            close(sock);
        }
    char Status[] = "HTTP/1.1 200 OK\r\n";                
    char server[]="Server:Linux Web Server\r\n";           
    char cnt_len[] = "Content-length:2048\r\n";
    char Content_type[SMALL_SIZE];                     
    sprintf(Content_type, "Content-type:%s\r\n\r\n", type); 
    
    //status.... content_type 까지 전달
    write(sock, Status, strlen(Status));                    
    write(sock, server, strlen(server));
    write(sock, cnt_len, strlen(cnt_len));
    write(sock, Content_type, strlen(Content_type));
    read(fp, buf, 2048);                    
    write(sock, buf, 2048);                   
    close(sock);                  
}
void error_message(char *arg)
{
    perror(arg);             
    exit(1);
}
