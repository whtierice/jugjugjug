// 서버
#include "csapp.h"

void echo(int connfd);
// connfd와의 통신을 처리하는 함수가 따로 정의되어 있다는 선언
int main(int argc, char **argv)
{
    int listenfd, connfd;
    // listenfd: 클라이언트 연결 요청을 기다리는 리스닝 소켓
		// connfd: 연결이 수락된 후, 클라이언트와 통신을 위한 연결 소켓
    socklen_t clientlen;
    // 연결된 클라이언트 소켓 주소의 크기를 기록하는 데 활용됨.
    
    struct sockaddr_storage clientaddr;  /* Enough space for any address */
    // 클라이언트 주소 정보를 담을 구조체
    // 모든 타입을 담을 수 있을만큼 크고,
    // 구조체의 사이즈를 같이 전달, 그 이상 읽을 일이 없음.
    char client_hostname[MAXLINE], client_port[MAXLINE];
    // 접속한 클라이언트의 호스트 이름과 포트번호를 문자열로 저장할 버퍼

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]);
    // 지정된 포트에 대해 리스닝 소켓을 생성
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        // accpet 호출 전, 클라이언트 주소의 크기를 지정
        // 마찬가지로 어떤 주소체계에도 대응하기 위해 가장 주소형식 중 가장 큰 값과 일치함.
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);  // 연결 수락
        //  클라이언트가 접속해오면 그 연결을 수락하고, 그 연결용 소켓을 connfd에 반환
				//  커널이 클라이언트의 주소정보를 clientaddr에 저장함.

        // 클라이언트가 방문한 사람이라면,
        // 커널은 "누구 왔는지 확인해줄게" 하면서
        // 당신이 준비한 메모장(clientaddr)에 방문자의 이름과 주소를 써줍니다.
        // 그 메모장을 들고 얘기하려면 connfd를 사용합니다.


        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        // 클라이언트 주소를 사람이 읽을 수 있는 형식으로 변환
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        // 누가 접속했는지 표시
        echo(connfd);           // 클라이언트 처리
        Close(connfd);          // 연결 종료
    }
    //무한으로 루프를 돌며 다음 클라이언트의 요청을 기다림
}
