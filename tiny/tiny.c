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
void serve_static(int fd, char *filename, int filesize, char *method);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
	    fprintf(stderr, "usage: %s <port>\n", argv[0]);
	    exit(1);
    }

    listenfd = Open_listenfd(argv[1]); // 포트 주소를 넘기면 listen descriptor
    while (1) {
      clientlen = sizeof(clientaddr);
      
      // accept 함수는 1.듣기 식별자, 2. 소켓주소구조체의 주소, 주소의 길이를 인자로 받는다
      connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept

      //Getaddrinfo는 호스트 이름, 호스트 주소, 서비스 이름, 포트 번호의 스트링 표시를 소켓 주소 구조체로 변환한다
      //Getnameinfo는 위를 반대로 소켓 주소 구조체에서 스트링표시로 변환한다
      Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);

      printf("Accepted connection from (%s, %s)\n", hostname, port);
      doit(connfd);                                             //line:netp:tiny:doit
      //트랜잭션이 수행된 후 자신 쪽의 연결끝을 닫는다
      Close(connfd);                                            //line:netp:tiny:close
    }
}

void echo(int connfd){
  size_t n;
  char buf[MAXLINE];
  rio_t rio;

  Rio_readinitb(&rio, connfd);
  while((n == Rio_readlineb(&rio, buf, MAXLINE)) != 0){
    printf("server received %d bytees\n", (int)n);
    Rio_writen(connfd, buf, n);

  }
}
/* $end tinymain */

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int fd) 
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];

    // rio_realineb를 위해 rio_t타입의 읽기 버퍼를 선언
    rio_t rio;

    /* Read request line and headers */

    // Rio = Robust I/O
    // rio_t 구조체를 초기화 해준다

    //strcmp() 문자열 비교 함수, 
    //strcasecmp() 대소문자를 무시하는 문자열 비교 함수 
    //strncasecmp() 대소문자를 무시하고, 지정한 길이만큼 문자열을 비교하는 함수

    Rio_readinitb(&rio, fd);

    /* Rio_readlineb(그림 10.8)를 통해 요청 라인을 읽어들이고, 분석한다. 70 +4 line */
    if (!Rio_readlineb(&rio, buf, MAXLINE))  //요청리인 읽고 분석
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);       //추가 코드
    if (strcasecmp(method, "GET") &&  strcasecmp(method, "HEAD")) {  // Tiny는 GET메소드만 지원. 만약 다른 메소드(like POST)를 요청하면. strcasecmp : 문자열비교.                   
        clienterror(fd, method, "501", "Not Implemented", "Tiny does not implement this method");
        return;  //그 후 연결을 닫고 다음 요청을 기다림. 그렇지 않으면 읽음
    }

    /* GET method라면 읽어들이고, 다른 요청 헤더들을 무시한다. */       //line:netp:doit:endrequesterr
    read_requesthdrs(&rio);                                  //line:netp:doit:readrequesthdrs


    /* Parse URI from GET request */
    /* URI 를 파일 이름과 비어 있을 수도 있는 CGI 인자 스트링으로 분석하고, 요청이 정적 또는 동적 컨텐츠를 위한 것인지 나타내는 플래그를 설정한다.  */
    is_static = parse_uri(uri, filename, cgiargs);       //line:netp:doit:staticcheck

    //파일이 디스크상에 있지 않다면 에러메세지를 즉시 클라이언트에게 보내고 메인루틴으로 리턴
    if (stat(filename, &sbuf) < 0) {    //요청이 정적 또는 동적 컨텐츠를 위한 것인지 나타내는 플래그 버퍼에 파일네임을 넘김                 
	    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file"); //만일 파일이 디스크상에 존재하지 않으면 클라이언트에게 에러메세지 보냄
	    return;
    }                                                    //line:netp:doit:endnotfound

    if (is_static) { /* Serve static content */
    // 정적컨텐츠이고 , 이 파일이 보통 파일인지, 읽기 권한을 가지고 있는지 검증          
	    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) { // sbuf.st_mode : sbuf의 내용 중 st_mode의 값(어떤 타입의 파일인지)
	      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
	      return;
	    }
  
    //정적 컨텐츠를 클라이언트한테 제공
	    serve_static(fd, filename, sbuf.st_size, method);        //line:netp:doit:servestatic
    }
    else { /* Serve dynamic content */
    //실행 가능한 파일인지 검증
	    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) { //line:netp:doit:executable
	      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
	      return;
	    }

      //동적 컨텐츠를 클라이언트에게 제공
	    serve_dynamic(fd, filename, cgiargs, method);            //line:netp:doit:servedynamic
    }
}
/* $end doit */

/*
 * read_requesthdrs - read HTTP request headers
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);

    while(strcmp(buf, "\r\n")) {          //헤더의 막줄은 비어있기때문에 \r\n만 buf에 담겨있다면 while문 종료
	    Rio_readlineb(rp, buf, MAXLINE);
	    printf("%s", buf);
    }
    return;
}
/* $end read_requesthdrs */

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs) 
{
  char *ptr;

  if (!strstr(uri, "cgi-bin")) {  /* 정적컨텐츠를 위한 */ //strstr으로 cgi-bin이 들어있는지 확인하고 양수값을 리턴하면 dynamic content를 요구하는 것이기에 노해당
    strcpy(cgiargs, "");        //CGI 인자 스트링을 지운다
    strcpy(filename, ".");                           //URI를 리눅스 경로 이름으로 바꿈
    strcat(filename, uri);                           //line:netp:parseuri:endconvert1

    if (uri[strlen(uri)-1] == '/')                   //URI가 '/'로 끝난다면
      strcat(filename, "home.html");                 //기본 파일 이름을 filename에 추가한다
    return 1;
  }
  else {  /* Dynamic content */                      //동적컨텐츠를 위한 요청이라면
    ptr = index(uri, '?');                           //물음표가 들어있으면 index 함수는 문자열에서 특정문자의 위치를 반환
    if (ptr) {
      strcpy(cgiargs, ptr+1);         //인자로 주어진 값들을 cgiargs 변수에 넣고
      *ptr = '\0';                    //ptr 초기화
    }
    else 
      strcpy(cgiargs, "");                         //없으면 안넣음

    strcpy(filename, ".");                           //나머지 URI 부분을상대 리눅스 파일잉름으로 변환
    strcat(filename, uri);                           //이어붙이는 함수 파일네임에 uri를 붙인다
    return 0;
  }
}
/* $end parse_uri */

/*
 * serve_static - copy a file back to the client 
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize, char *method) {
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF], *fbuf;

  /* Send response headers to client */
  get_filetype(filename, filetype); // 파일 이름의 접미어 부분 검사해서 파일 타입 결정
  sprintf(buf, "HTTP/1.0 200 OK\r\n"); // 클라이언트에 응답 줄과 응답 헤더를 보낸다
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype); // 빈 줄 한 개가 헤더를 종료하고 있음
  /* writen = client 쪽에 */
  Rio_writen(fd, buf, strlen(buf)); // 요청한 파일의 내용을 연결 식별자 fd로 복사해서 응답 본체를 보낸다 // 버퍼를 옮김
  /* 서버 쪽에 출력 */
  printf("Response headers:\n"); 
  printf("%s", buf);

  if (!strcasecmp(method, "HEAD")) // 같으면 0(false). 다를 때 if문 안으로 들어감
    return; // void 타입이라 바로 리턴해도 됨(끝내라)

  /* Send response body to client */
  srcfd = Open(filename, O_RDONLY, 0); // 열려고 하는 파일의 식별자 번호 리턴. filename을 오픈하고 식별자를 얻어온다
                                // ㄴ 0 : 읽기 전용이기도하고, 새로 파일을 만드는게 아니니 Null처럼 없다는 의미..없어도 됨
  // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // mmap : 요청한 파일을 가상메모리 영역으로 매핑한다
  // // mmap 호출시 위에서 받아온 모든 요청 정보들(srcfd)을 전부 매핑해서 srcp로 받는다(포인터)
  // Close(srcfd); // srcfd 내용을 메모리로 매핑한 후에 더 이상 이 식별자 필요X, 파일을 닫는다. 안 닫으면 메모리 누수 치명적
  // Rio_writen(fd, srcp, filesize); // 실제로 파일을 클라이언트에게 전송. // srcp내용을 fd에 filesize만큼 복사해서 넣는다
  // Munmap(srcp, filesize); // 매핑된 srcp 주소를 반환한다. 치명적인 메모리 누수를 피한다 
  // // mmap-munmap은 malloc-free처럼 세트
  
  /* 숙제문제 11.9 */ 
  fbuf = malloc(filesize); //filesize 만큼의 가상 메모리(힙)를 할당한 후(malloc은 아무것도 없는 빈 상태에서 시작) , Rio_readn 으로 할당된 가상 메모리 공간의 시작점인 fbuf를 기준으로 srcfd 파일을 읽어 복사해넣는다.
  Rio_readn(srcfd, fbuf, filesize); // srcfd 내용을 fbuf에 넣는다(버퍼에 채워줌)
  Close(srcfd); // 윗줄 실행 후 필요 없어져서 닫아준다 // 양 쪽 모두 생성한 파일 식별자 번호인 srcfd 를 Close() 해주고
  Rio_writen(fd, fbuf, filesize); // Rio_writen 함수 (시스템 콜) 을 통해 클라이언트에게 전송한다 
  // fbuf를 fd에다가 넣는다(fbuf는 사실 포인터. 걔를 밀면서 writen, 미는 애는 새로 선언한 usrbuf)
  free(fbuf); // Mmap은 Munmap, malloc은 free로 할당된 가상 메모리를 해제해준다.
}


/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
	strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
	strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
	strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
	strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".mpeg"))
	strcpy(filetype, "video/mpeg");
    else if (strstr(filename, ".mp4"))
	strcpy(filetype, "video/mp4");
    else 
	strcpy(filetype, "text/plain");
}  
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method) {
  char buf[MAXLINE], *emptylist[] = { NULL };

  /* Return first part of HTTP response */
  // 클라이언트에게 성공을 알려주는 응답 라인을 보내는 것으로 시작
  sprintf(buf, "HTTP/1.0 200 OK\r\n"); 
  Rio_writen(fd, buf, strlen(buf));  // fd에 버퍼 넣는다
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (!strcasecmp(method, "HEAD")) // 같으면 0(false). 다를 때 if문 안으로 들어감
    return; // void 타입이라 바로 리턴해도 됨(끝내라)

  // 응답의 첫 번째 부분을 보낸 후, 새로운 자식 프로세스를 fork한다
  if (Fork() == 0) { /* Child */ 
    /* Real server would set all CGI vars here */
    // cgiargs에 arguments 2개가 들어있음 (parse에서 물음표 기준으로 2개로 나눴음 strcpy)
    setenv("QUERY_STRING", cgiargs, 1); // 환경변수 설정. QUERY_STRING 환경변수를 요청 URI의 CGI인자들로 초기화한다
    Dup2(fd, STDOUT_FILENO); /* Redirect stdout to client */ // 자식의 표준 출력을 연결 파일 식별자로 재지정
    Execve(filename, emptylist, environ); /* Run CGI program */ // 그 후 CGI프로그램 로드 후 실행
  }
  Wait(NULL); /* Parent waits for and reaps child */ // 부모는 자식이 종료되어 정리되는 것을 기다리기 위해 wait 함수에서 블록된다(대기하는 함수)
/* $end serve_dynamic */
}
/*
 * clienterror - returns an error message to the client
 */

/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}
/* $end clienterror */