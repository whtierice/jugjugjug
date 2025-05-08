// 메세지 되돌려주는 함수
#include "csapp.h"

void echo(int connfd)
// 클라이언트와 통신할 소켓 디스크립터(connfd)를 받아서 다시 돌려주는 함수.
{
    size_t n;
    // 받은 바이트 수
    char buf[MAXLINE];
    // 데이터를 저장할 버퍼
    rio_t rio;
    // robust I/O 시스템을 위한 구조체

    Rio_readinitb(&rio, connfd);
    // connfd를 rio 버퍼에 등록
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
    // 줄바꿈 문자(\n)를 기준으로 데이터를 읽음.
    // 읽어서 buf에 저장.
    // 리턴 값은 n에 읽은 바이트 수로 들어감.
    // EOF(연결 종료) 시 n == 0이 되어 루프 종료.
    {
        printf("server received %d bytes\n", (int)n);
        // 받은 바이트 수를 기록
        Rio_writen(connfd, buf, n);
        // 받은 내용을 connfd에 쓰기
    }
}