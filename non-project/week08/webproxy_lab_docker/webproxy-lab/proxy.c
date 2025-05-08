#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* User-Agent string (고정된 값) */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

#define MAXLINE 8192
#define MAXBUF  8192

void doit(int connfd);
void parse_uri(char *uri, char *hostname, char *path, int *port);
void build_http_request(char *http_req, char *hostname, char *path);

int main(int argc, char **argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        Close(connfd);
    }
}

void doit(int connfd) {
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char hostname[MAXLINE], path[MAXLINE], http_req[MAXBUF];
    int port;

    rio_t rio_client, rio_server;

    Rio_readinitb(&rio_client, connfd);
    Rio_readlineb(&rio_client, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);

    if (strcasecmp(method, "GET")) {
        printf("Proxy does not implement the method %s\n", method);
        return;
    }

    parse_uri(uri, hostname, path, &port);
    build_http_request(http_req, hostname, path);

    int serverfd = Open_clientfd(hostname, port);
    if (serverfd < 0) {
        fprintf(stderr, "Failed to connect to server %s\n", hostname);
        return;
    }

    Rio_readinitb(&rio_server, serverfd);
    Rio_writen(serverfd, http_req, strlen(http_req));

    size_t n;
    while ((n = Rio_readlineb(&rio_server, buf, MAXLINE)) != 0) {
        Rio_writen(connfd, buf, n);
    }

    Close(serverfd);
}

void parse_uri(char *uri, char *hostname, char *path, int *port) {
    *port = 80; // 기본 포트
    char *hostbegin = strstr(uri, "//") ? strstr(uri, "//") + 2 : uri;
    char *hostend = strpbrk(hostbegin, ":/");
    int hostlen = hostend ? hostend - hostbegin : strlen(hostbegin);
    strncpy(hostname, hostbegin, hostlen);
    hostname[hostlen] = '\0';

    if (hostend && *hostend == ':') {
        sscanf(hostend + 1, "%d%s", port, path);
    } else if (hostend && *hostend == '/') {
        strcpy(path, hostend);
    } else {
        strcpy(path, "/");
    }
}

void build_http_request(char *http_req, char *hostname, char *path) {
    sprintf(http_req,
            "GET %s HTTP/1.0\r\n"
            "Host: %s\r\n"
            "%s"
            "Connection: close\r\n"
            "Proxy-Connection: close\r\n"
            "\r\n",
            path, hostname, user_agent_hdr);
}
