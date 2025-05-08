/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  // 서버 소켓과 클라이언트 연결 소켓
  // 클라이언트 호스트 정보
  // 클라이언트 주소 구조체 길이
  // 클라이언트 주소 정보 저장

  /* Check command line args */
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  // 아마도
  // 서버역할을 위해 AI_PASSIVE가 되어있을것

  listenfd = Open_listenfd(argv[1]); // 지정한 포트에 대해 서버 소켓 생성 (리스닝 소켓 열기)

  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen); // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);  // 요청처리 함수
    Close(connfd); // 소켓 닫기
  }
}

void doit(int fd)
{
  int is_static;    // 정적/동적 콘텐츠 여부
  struct stat sbuf; // 파일 정보 저장 구조체
  char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  // 요청 URL에서 추출된 파일명, 인자
  rio_t rio;
  // Robust I/O 버퍼

  /* 요청 라인과 헤더 읽기 ---------------------------------------*/

  Rio_readinitb(&rio, fd);
  printf("HTTP 요청 읽기 전 초기화 완료.\n");
  // rio 구조체 초기화 (fd는 클라이언트 소켓)

  Rio_readlineb(&rio, buf, MAXLINE);
  printf("요청 읽기 완료. 커널버퍼에서 다 읽었지만, 아직 커널 버퍼에 요청 전체가 도착하지 않았을 수도 있음.\n");
  // 1. 커널 버퍼에서 최대 rio_buf 크기만큼 read()로 가져와서 rio_buf[]에 저장
  // 2. rio_buf[]에서 한 바이트씩 꺼내며 \n을 만날 때까지 사용자 버퍼(usrbuf)에 복사
  // 3. \n을 만나면 종료 (한 줄 완성), 아니면 반복
  // 4. 만약 \n을 못만난다면? 더 많은 데이터를 얻기 위해 blocking 상태로 read() 호출
  // 5. 그런데 커널에도 아직 데이터가 없다면?
  // read()는 blocking 상태로 대기
  // 클라이언트가 나머지 줄 데이터를 보내줄 때까지 멈춤
  sscanf(buf, "%s %s %s", method, url, version);
  // 요청 라인을 method, url, version으로 분리
  printf("%s %s %s\n", method, url, version);
  // 로그 출력

  if (strcasecmp(method, "GET"))
  { // GET 요청이 아닌 경우
    clienterror(fd, method, "501", "Not implemented",
                "Tiny does not implement this method"); // 오류 메시지 전송
    return;
  }

  read_requesthdrs(&rio);
  // 헤더들 무시 (읽기만 함)
  // 최소 구현을 위해 헤더는 파싱하지 않고 넘김.

  /* URI에서 파일 이름과 CGI 인자 추출 -----------------------------*/
  is_static = parse_uri(url, filename, cgiargs);
  // parse_uri()를 통해 정적/동적 요청을 구분함.
  // 또한 filename과 cgiargs 변수를 채움.
  // filename에는 파일 시스템에서 실제로 찾아야할 파일의 경로가 저장됨.

  if (stat(filename, &sbuf) < 0)
  // stat함수가 호출되면서 파일의 경로를 찾아가 파일의 정보를 확인한 뒤
  // sbuf에 파일의 다양한 속성 정보가 채워짐
  // 만약 그 파일이 없으면 -1을 반환/
  {
    clienterror(fd, filename, "404", "Not found",
                "Tiny couldn't find this file");
    return;
  }

  if (is_static)
  { // 정적 콘텐츠 처리
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
    // 권한은 파일에 설정되어있고 사용자가 권한에 해당하면 읽을 수 있음.
    { // 일반 파일인지 검사 + 사용자에게 읽기 권한 확인
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny couldn't read the file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size); // 정적 콘텐츠 전송
  }
  else
  { // 동적 콘텐츠 처리
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
    { // 실행 가능 여부 확인
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny couldn't run the CGI program");
      return;
    }
    serve_dynamic(fd, filename, cgiargs); // CGI 프로그램 실행
  }
}

void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF]; // HTTP 헤더용, HTML 본문용 버퍼

  /* HTML 형태의 오류 메시지 본문 구성 */
  sprintf(body, "<html><title>Tiny Error</title>"); // HTML 제목 설정
  sprintf(body + strlen(body), "%s<body bgcolor="
                               "ffffff"
                               ">\r\n",
          body);                                                                // 흰 배경 설정
  sprintf(body + strlen(body), "%s%s: %s\r\n", body, errnum, shortmsg);         // 예: "404: Not found"
  sprintf(body + strlen(body), "%s<p>%s: %s\r\n", body, longmsg, cause);        // 상세 오류 메시지 포함
  sprintf(body + strlen(body), "%s<hr><em>The Tiny Web server</em>\r\n", body); // 하단 서명 추가

  /* HTTP 응답 헤더 출력 */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  // 상태 줄 예: "HTTP/1.0 404 Not found"
  Rio_writen(fd, buf, strlen(buf));
  // 상태 줄 전송
  sprintf(buf, "Content-type: text/html\r\n");
  // 콘텐츠 타입 지정
  Rio_writen(fd, buf, strlen(buf));
  // 콘텐츠 타입 전송
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  // 본문 길이 및 헤더 종료
  Rio_writen(fd, buf, strlen(buf));
  // 길이 정보 전송

  Rio_writen(fd, body, strlen(body));
  // 실제 HTML 오류 본문 전송
}

// void read_requesthdrs(rio_t *rp)
// {
//   char buf[MAXLINE];

//   rio_readlineb(rp, buf, MAXLINE);
//   // 버퍼에 입력 채우기 용, 만약 이게 없다면,
//   // 버퍼에 쓰레기 값이 있는 상태로 strcmp를 하기 때문에,
//   // 예상치 못한 오류가 발생할 가능성이 있음.
//   while (strcmp(buf, "\r\n"))
//   {
//     rio_readlineb(rp, buf, MAXLINE);
//     printf("%s", buf);
//   }

//   return;
// }

// do while 버전
void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];

  do
  {
    rio_readlineb(rp, buf, MAXLINE); // 먼저 읽고
    printf("%s", buf);               // 출력
  } while (strcmp(buf, "\r\n")); // 빈 줄이 아닐 때까지

  return;
}

int parse_uri(char *uri, char *filename, char *cgiargs)
{
  char *ptr;

  // "cgi-bin"이 포함되어 있지 않으면 → 정적 콘텐츠 요청
  if (!strstr(uri, "cgi-bin"))
  { /* Static content */
    strcpy(cgiargs, "");
    // CGI 인자는 없음 (빈 문자열로 초기화)
    strcpy(filename, "."); // 상대 경로 시작을 현재 디렉토리로 설정 (예: "./")
    strcat(filename, uri); // uri 붙이기 (예: "./index.html")

    // 만약 URI가 '/'로 끝나면, 기본 파일 이름인 "home.html"을 붙여줌
    if (uri[strlen(uri) - 1] == '/')
      strcat(filename, "home.html");

    return 1; // 정적 콘텐츠라는 의미로 1 반환
  }

  // 동적 콘텐츠 요청 처리
  else
  { /* Dynamic content */
    ptr = index(uri, '?');
    // URI에서 '?' 문자의 위치를 찾음
    if (ptr)
    {
      // '?'가 존재하면
      strcpy(cgiargs, ptr + 1);
      // '?' 다음에 나오는 문자열을 CGI 인자로 복사
      *ptr = '\0';
      // '?' 자리를 널 문자로 바꿔 URI를 자름 → 앞부분만 파일 경로로 사용
    }
    else
    {
      strcpy(cgiargs, ""); // '?'가 없으면 CGI 인자는 없음
    }

    strcpy(filename, ".");
    // 상대 경로 시작 (현재 디렉토리)
    strcat(filename, uri);
    // 잘린 URI를 파일 경로로 붙임 (예: "./cgi-bin/add.pl")

    return 0; // 동적 콘텐츠라는 의미로 0 반환
  }
}

// 정적 콘텐츠를 클라이언트에게 전송하는 함수
void serve_static(int fd, char *filename, int filesize)
{
  int srcfd;
  // 클라이언트에게 보낼 정적 파일을 열었을 때, 그 파일을 가리키는 정수 값
  char *srcp, filetype[MAXLINE], buf[MAXBUF];
  // mmap으로 파일을 메모리에 매핑한 주소를 저장
  // MIME 타입을 저장하기 위한 버퍼
  // 응답 헤더를 담는 데 사용됨.

  /* -------------------- 응답 헤더 생성 및 전송 -------------------- */
  get_filetype(filename, filetype);
  // 파일 이름에서 MIME 타입 결정 (예: text/html)

  // 상태 줄과 헤더 필드 작성
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  // 상태 줄
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  // 서버 정보
  sprintf(buf, "%sConnection: close\r\n", buf);
  // 커넥션 종료 명시
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  // 콘텐츠 길이
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  // 콘텐츠 타입
  Rio_writen(fd, buf, strlen(buf));
  // 클라이언트에게 응답 헤더 전송

  printf("Response headers:\n");
  printf("%s", buf);
  // 서버 측 로그 출력

  /* -------------------- 응답 본문 전송 (파일 내용) -------------------- */
  srcfd = Open(filename, O_RDONLY, 0);
  // 파일 열기 (읽기 전용)
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  // 파일을 메모리에 매핑
  // srcp = malloc(filesize);
  // if (srcp == NULL){
  //   Close(srcfd);
  //   return -1;
  // }
  // rio_readn(srcfd,srcp,filesize);

  Close(srcfd);
  // 파일 디스크립터는 더 이상 필요 없으므로 닫음

  Rio_writen(fd, srcp, filesize);
  // free(srcp);
  // 매핑된 파일 내용을 클라이언트에게 전송
  Munmap(srcp, filesize);
  // 매핑 해제 (자원 정리)
}

void get_filetype(char *filename, char *filetype)
{
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".mp4"))
    strcpy(filetype, "video/mp4");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else
    strcpy(filetype, "text/plain");

  // 파일 이름을 보고 .html, .png, .jpg 같은 확장자 문자열이 포함되어 있는지 검사

  // 해당하는 MIME 타입 문자열을 filetype에 복사
}

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
  char buf[MAXLINE], *emptylist[] = {NULL};
  // buf: 클라이언트에 보낼 메시지를 저장할 버퍼
  // emptylist: execve 호출 시 사용할 빈 argv 리스트
  // 왜냐하면 execve는 반드시 argv 배열을 요구함. 배열 형식도 갖추지 않은 채로 NULL을 넘기면 문제가 발생할 수 있음.

  /* 클라이언트에 응답 헤더 첫 부분 전송 */
  sprintf(buf, "HTTP/1.0 200 OK\r\n"); // HTTP 상태 줄 작성
  Rio_writen(fd, buf, strlen(buf));    // 클라이언트로 전송

  sprintf(buf, "Server: Tiny Web Server\r\n"); // 서버 정보 헤더 작성
  Rio_writen(fd, buf, strlen(buf));            // 클라이언트로 전송

  if (Fork() == 0)
  // 자식프로세스를 생성하고 생성된 자식만 실행하는 코드.
  { // 자식 프로세스인 경우
    // CGI 프로그램이 참조할 환경 변수 설정
    setenv("QUERY_STRING", cgiargs, 1); // QUERY_STRING=cgiargs 형식으로 설정

    // 표준 출력을 클라이언트 소켓으로 리디렉션
    Dup2(fd, STDOUT_FILENO); // STDOUT을 fd로 연결 → printf 결과가 클라이언트로 감

    // CGI 프로그램 실행 (현재 프로세스를 filename으로 대체)
    Execve(filename, emptylist, environ);
    // emptylist: argv 인자 없음, environ: 현재 환경 변수 전달
  }

  // 부모 프로세스는 자식이 끝날 때까지 기다림 (좀비 프로세스 방지)
  Wait(NULL);
}
