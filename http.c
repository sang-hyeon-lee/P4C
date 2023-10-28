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

void *request(void *sock);                      // client의 요청을 관리하는 함수이다.
void error_message(char *arg);                  // 오류가 발생 시 호출할 함수이다.
void response(char *type, int sock, int i);     // 서버에서 직접 응답을 보내는 함수이다.
char *ret_type(int i);                          // 타입을 전달하는 함수이다. (text.html)

int main(int argc, char *argv[])                
{
    int serv_sock, clnt_sock;                   // 서버 소켓과 클라이언트 소켓을 담을 변수
    struct sockaddr_in serv_addr, clnt_addr;    // 서버 주소와 클라이언트 주소를 담을 구조체
    int clnt_addr_size;                         // 클라이언트 주소의 길이를 담을 함수
    pthread_t gg;                               // 쓰레드 함수
    if(argc != 2)                               // 포트를 입력하지 않았을 시
    {
        printf("please port : %s\n", argv[0]);  // 파일 이름 출력 후
        exit(1);                                // 종료
    }

    //serv_addr 초기화
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);         // socket함수를 이용한 소켓 생성, ipv4, tcp/ip로 생성
    memset(&serv_addr, 0, sizeof(serv_addr));            // sin_zero[8] 초기화를 위한 전체 초기화 
    serv_addr.sin_family = AF_INET;                      // ipv4
    serv_addr.sin_port = htons(atoi(argv[1]));           // 포트번호를 빅 엔디안 방식으로 변환해서 할당
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);       // 자동으로 ip주소 할당
    
    if(bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)       //bind 실패 시
        error_message("bind error");
    
    if(listen(serv_sock, 10) == -1)                      //listen 실패 시
        error_message("listen error");
    
    while(1)
    {
        clnt_addr_size = sizeof(clnt_addr);                // 클라이언트 주소 구조체 크기만큼 할당
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);  //clnt_addr에는 client 주소정보가 들어가 있다.
        if (clnt_sock == -1)
            error_message("accept error");
        pthread_create(&gg, NULL, request, &clnt_sock);    //쓰레드 생성 후 request 호출 인자 값으로 클라이언트 소켓 전송
        pthread_detach(gg);                                //종료 시 쓰레드 제거
    }
    close(serv_sock);
    return 0;
}
void *request(void *sock)                 // 쓰레드 생성 후!
{
    char title[SMALL_SIZE]={0,};          // http의 start line 부분을 읽어오기 위한 배열 선언
    char type[15] = {0,};                 // content_type을 전달 받을 변수
    int clnt_sock = *(int *)sock;         // clnt_sock을 생성하고 형변환을 통해 받고 있다. void (*sock)이라 (int *)형을 변환하는 것이다.

    read(clnt_sock, title, 30);           // http request의 start_line을 읽는다.
    if(strstr(title, "html") != NULL)     // html을 요청하는 것이라면
    {
        strcpy(type, ret_type(1));        // type에 html 반환
        response(type, clnt_sock, 1);     // 숫자 1을 매개변수로 받고 서버 응답 함수로 이동
    }
    else if(strstr(title, "jpg") != NULL)  // jpg를 요청하는 것이라면
    {
        strcpy(type, ret_type(1));         // html을 반환받고
        response(type, clnt_sock, 2);      // 숫자 2를 매개변수로 받고 서버 응답 함수로 이동
    }
    else
        strcpy(type, ret_type(1));         // 그 외 다른것을 요구 받는다면
        response(type, clnt_sock, 3);      // 숫자 3을 매개변수로 받고 서버 응답 함수로 이동
}
char* ret_type(int i)
{
    if (i == 1)
        return "text/html";               // contecnt_type인 text/html 반환
}
void response(char *type, int sock, int i)
{
    char buf[BIG_SIZE] = {0,};                             // 파일 내용을 읽을 buf -> 2048만큼 읽는다
    int fp;                                                // 파일 디스크립터를 전달받을 변수 생성
    if (i == 1)                                            // html을 요구했다면
        fp = open("index.html", O_RDONLY);                 // index.html을 연다
    else if (i == 2)                                       // jpg를 요구했다면
        fp = open("index2.html", O_RDONLY);                // jpg의 경로를 가지고 있는 index2.html을 연다.
    else if (i == 3)                                       // 그 외 다른것을 요청 했다면
        {
            char Status[] = "HTTP/1.1 404 Not found\n";    //404 error발생
            write(sock, Status, strlen(Status));
            close(sock);
        }
    char Status[] = "HTTP/1.1 200 OK\r\n";                 // 정상적으로 메시지를 잘 받았음을 response
    char server[]="Server:Linux Web Server\r\n";           
    char cnt_len[] = "Content-length:2048\r\n";
    char Content_type[SMALL_SIZE];                         //content type은 위에서 전달받은 text/html
    sprintf(Content_type, "Content-type:%s\r\n\r\n", type);  //\r\n\r\n 두번을 하는 이유는 헤더와 바디를 구분하기 위해서이다. 헤더가 끝나면 blank 공간을 남기고 바디가 시작한다.
    
    //status.... content_type 까지 전달
    write(sock, Status, strlen(Status));                    
    write(sock, server, strlen(server));
    write(sock, cnt_len, strlen(cnt_len));
    write(sock, Content_type, strlen(Content_type));
    read(fp, buf, 2048);                       // 바디 부분에 전달할 html를 읽고
    write(sock, buf, 2048);                    // 바디 부분에 html을 전달한다
    close(sock);                  
}
void error_message(char *arg)
{
    perror(arg);             // 에러 메시지를 호출!
    exit(1);
}