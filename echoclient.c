#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    int s;
    struct sockaddr_in server_addr;
    int addr_len;
    char msg[1001]; //11byte
    int len;
    int read_len;

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
        return 0;
    }

    //주소 구조체
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(54321); //16비트(2byte) 빅엔디언으로 바꿔서

    inet_aton("127.0.0.1", &server_addr.sin_addr); //서버의 Ip주소를 넣는다
    memset(&(server_addr.sin_zero), 0, 8);  //8byte를 0으로 초기화

    /*connect() 동작 - 3wayhandshake   
    connect함수 호출하면 sink라는 메세지가 간다 그리고 서버가 리슨 상태면 싱크를 받는다 그리고 다시 클라이언트에게 싱크메세지와 ack을 보낸다
    싱크 메세지와 ack을 받으면 커넥트함수의 리턴값을 내뱉는다 리턴을 하면서 서버쪽에 ack을 보낸다(ack segment can include data) */
    // 서버의 주소를 보고싶으면 ifconfig 127.0.0.1 -> loopback 이라고 해서 자기자신한테 다시 돌아오는 ip
    // netstat -ntlp | grep :80

    if(connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        return 0;
    }

    while(1){
        printf("> ");
        //scanf는 line단위로 입력이 안된다
        fgets(msg, 1000, stdin);
        len = write(s, msg, strlen(msg));
        if(len == -1){
            perror("write");
            close(s);
            return 0;
        }
        while(len > 0){  // 데이터의 일부만 읽었을 경우 계속 read()호출

            read_ len = read(s, msg, 1000); //읽을 때 이만큼 읽는다~
            if(len == -1){
                perror("read");
                close(s);
                return 0;
            }
            if(len == 0){

                printf("Disconnected\n");
                break;
            }
            len -= read_len;
            msg[read_len] = 0;
            print("%s", msg);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
        }
        // msg[len] = 0;
        // printf("echo: %s\n", msg);
    }  
    
    close(s);
    return 0;
}