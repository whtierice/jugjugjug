// 클라이언트

#include "csapp.h"

int main(int argc, char **argv)
// 커맨드라인에서 실행될 때 host와 port를 입력받는 구조.
// 문자열의 배열이기 때문에 이중포인터로 받는다. char *argv[]와 같은 의미.
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    // 버퍼의 최대크기는 정의되어있음.
    rio_t rio;
    // robust I/O를 위한 구조체
    // 버퍼내에서 읽을 위치에 대한 포인터, 버퍼에 남은 바이트 수 대상 파일 디스크립터 등이 저장되어있음.

    if (argc != 3) // (실행코드) (host)(port)처럼 실행코드 포함 3개 인자가 들어오지 않으면 에러 출력
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    host = argv[1];
    port = argv[2];
    // 사용자가 넣은 인자를 host와 port에 저장.

    clientfd = Open_clientfd(host, port);
    // 서버와의 연결을 생성하여 클라이언트 소켓 디스크립터를 얻음.
    // 내부적으로 socket, getaddrinfo, connect 등을 호출.
    Rio_readinitb(&rio, clientfd);
    // Robust I/O 버퍼를 초기화. clientfd를 사용해 입출력을 안정적으로 처리하기 위함
    // 읽을 대상인 소켓 파일 디스크립터를 인자로 전달

    while (Fgets(buf, MAXLINE, stdin) != NULL)
    {
        // 표준입력(stdin)에서 한 줄(개행 문자 포함)을 읽어 buf에 저장한다.

        // 표준입력은 일반적으로 키보드 입력에 연결되어 있어,
        // 사용자가 키보드로 입력한 값을 읽는다는 의미이다.

        // Fgets 함수는 읽은 한줄을 버퍼에 저장한다.
        // 사용자 입력이 없고 Ctrl+D가 눌리면 EOF가 발생해 Fgets는 NULL을 반환하고 루프를 빠져나간다.

        Rio_writen(clientfd, buf, strlen(buf));
        // 입력한 내용을 clientfd 소켓을 통해 서버로 전송.
        Rio_readlineb(&rio, buf, MAXLINE);
        // 서버에서 한 줄 응답을 읽어온다. (echo된 결과)
        Fputs(buf, stdout);
        // 서버가 보낸 응답을 표준 출력으로 다시 보여준다.
    }

    // 입력루프를 빠져나가면 프로그램이 더이상 하는 게 없이 종료된다.
    Close(clientfd);
    //  운영체제에게 빌려 쓴 소켓/파일 등을 반납
    exit(0);
    // exit(0)은 프로세스를 정상 종료시킨다.
}


