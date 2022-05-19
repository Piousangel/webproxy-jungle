#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    int s, client_s;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int addr_len;

    char buf[1000];
    int len;

    //domain : 어떤 영역에서 통신할 것인지에 대한 영역을 지정해 줍니다.
    //type : 어떤 타입의 프로토콜을 사용할 것인지에 대해 설정 하는 것
    // 0
    // 기본적으로 소켓은 프로토콜, ip주소, 포트 넘버로 정의된다
    // 포트란 같은 컴퓨터 내에서 프로그램을 식별하는 번호
    // 소켓 동신이란 서버와 클라이언트가 계속 연결을 유지하는 양방향 통신 ( 실시간으로 데이터를 주고받는 상황이 필요한 경우에 사용된다)
    // 실시간 동영상 스트리밍 올라인 게임등과 같은 경우 자주사용
    //bind을 할때는 주소 구조체가 필요하다


    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == -1){
        perror("socket");
    }

    //주소 구조체
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(54321); //16비트(2byte) 빅엔디언으로 바꿔서
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(server_addr.sin_zero), 0, 8);  //8byte를 0으로 초기화

    if(bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        perror("bind");
        return 0;
    }

    /*연결요청을 대기할 소켓, 서버에 접속요청이 여러가지가 오면 연결이 밀려 바로 처리못하기 때문에
    큐를 사용해서 통신, 데이터를 주고받을 때 양쪽속도가 다르면 무조건 큐가 필요함 버퍼라고 부름
    운영체제 디폴트로 1000이상이 들어있음 그래서 백로그 넣어줘도 무시할 때도 많다고 함*/
    if(listen(s, 5) == -1 ) {
        perror("listen");
        return 0;
    }

    //sudo apt install net-tools
    //netstat -pa --tcp
    // listen하면 시스템 함수로 확인할 수 있음 
    // system("netstat -pa --tcp");
    /* 서버가 listen을 하고 기다리다가 accept()함수가 실행되면 접속이 된다고 생각하는데
    그렇지 않고 리슨을 하고 포트가 열려있으면 accept를 부르던지 말던지 상관안하고 운영체제
    리눅스 유닉스가 접속요청이 오면 무조건 연결을 시켜준다 커넥트가 되면 새로 생성되는 소켓이
    나온다 그 소켓들이 큐에 쌓인다 이때 accept를 호출할 경우 큐(버퍼)에 쌓인 접속이 된 소켓들을
    하나씩 꺼내온다 즉. accept함수가 연결을 시켜주는게 아니라 버퍼에서 소켓을 가져오는 함수다 
    그리고 대기중인 요청이 없을 때 block이된다(리턴을 안한다)10*/

    // int accpet(int s, struct sockaddr *addr, soklen_t *addrlen)
    // s는 bind까지했던 그 소켓, addr은 클라이언트쪽 주소, c는 하나의 값만 리턴가능한데, 여러개의 리턴값이 필요할 경우 이렇게 매개변수로 둘  수있다)
    // int len = sizeof(scokaddr) , socklen_t &len



    addr_len = sizeof(struct sockaddr_in);

    client_s = accept(s, ((struct sockaddr *)&client_addr), &addr_len); //클라이언트 연결요청이 오면 block이 풀린다
    if(client_s == -1){
        perror("accept");
        return 0;
    }
    // system("netstat -pa --tcp");
    /* 연결해제 클라이언트, 서버 둘다 가능 4wayhadnshake 수행
    descriptor reference counts가 0이 되어야 FIN 전송 -> 예를 들어 파일디스크립터를 2개의 파일이 읽고 있으면 파일이 읽고있는지 여부로 레퍼런스카운터를 둬서 확인한다
    int close(int sockfd; 파라미터 sockfd: 닫을 소켓, 

    동작원리 - 4wayhandshake
    reference count 가 0이 되서 클라이언트가 close()함수를 호출하면 서버에 FIN명령어를 보낸다 그럼 서버가 끊어야 되는구나 알고 
    액을 보낼려고 하는데 서버 입장에서는 아직 끊을 준비가 안되었으면 액을 보내면서 계속 데이터를 클라이언트에게 보낸다
    그럼 클라이언트는 FIN_WAIT를 한다 그리고 서버쪽에서 데이터를 다 보냈으면 FIN N을 클라이언트에게 전송하고 그것을 받은 클라이언트는
    FIN N+1을 다시 서버에게 보내면서 close()가 이루어 진다.
    근데 서버에서돋 보낼 데이터가 없으면 ack과 FIN_N을 같이보내 3wayhandshake만에 close()할수있다.
    */

    /*send, recv함수와 write(), read()함수의 기능은 똑같지만 소켓통신에서는 send, recv함수를 많이 사용한다.
    추가된점은 flag를 이용하여 옵션을 설정할 수 있다 default : 0 
    return value = 성공시 송수신한 실제 바이트 수 */

    // *msg = "Hello World\n" 11byte를 보내도 되고 12byte를 보내도 된다. 문자열을 보낼때는 \n 널문자열로 끝을 알수있다? 기때문에 \n찍어주는게 좋다 파일은 노상관 
    
    //처음에 만든 소켓을 집어 넣는게 아니라 accept를 하고 난 다음의 client_s를 read함수의 매개변수로 읽는다 
    len = recv(client_s, buf, 1000, 0);
    if(len == -1){
        perror("read");
        return 0;
    }
    // buf[len]   = '\0';  // buf[len]의 위치는 알죵?
    printf("len : %d\n", len);
    printf("%s\n", buf);
    close(client_s);

    return 0;
}