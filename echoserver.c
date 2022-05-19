#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>


/* nc -> 서버를 열수도 있고 패킷을 만들 수도 있다 
nc하고 포트 번호를 써준다 그리고 listen을 할꺼면 -l 옵션을 붙혀준다 -> nc 54321 -l 스스로 포트를 만들고 접속을 기다린다 
이쪽 포트로 오는 메세지를 화면에 다 출력해준다 
클라이언트 단에서는 nc 서버의 ip주소 54321 -> nc 127.0.0.1 54321 하고 아무거나 쳐보세요 치는게 패킷에 저장되어 서버, 클라이언트로 간다
예를 들어 서버를 만들고 client가 없을 때 흉내를 내고 싶으면 위에 과정처럼 해볼 수 있다.
*/

int main()
{
    int s, client_s;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int addr_len;

    char buf[1000];
    int len;

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

    if(listen(s, 5) == -1 ) {
        perror("listen");
        return 0;
    }

    addr_len = sizeof(struct sockaddr_in);

    while(1){
        client_s = accept(s, ((struct sockaddr *)&client_addr), &addr_len);
        if(client_s == -1){
            perror("accept");
            return 0;
        }

        if(fork() == 0){
            close(s);

            while(1)
        }
    }

    // client_s = accept(s, ((struct sockaddr *)&client_addr), &addr_len); //클라이언트 연결요청이 오면 block이 풀린다
    // if(client_s == -1){
    //     perror("accept");
    //     return 0;
    // }
    // while(1){
    //     //read
    //     len = read(client_s, buf, 1000);
    //     if(len == -1){
    //         perror("read");
    //         return 0;
    //     }
    //     else if(len == 0){
    //         printf("Disconneted\n");
    //         break;
    //     }

    //     // buf[len] = 0 아스키코드 null 밑이랑 같은 말이다 
    //     buf[len] = '\0'; //마지막칸에 null 박고 
    //     printf("recv: %s\n", buf);
    //     //write
    //     len = write(client_s, buf, len); //받을 때도 len만큼 받았기 때문에 보낼 때도 len만큼 보내면 된다.
    //     if(len == -1){
    //         perror("write");
    //         return 0;
    //     }
    // }

    // close(client_s);
    close(s);

    return 0;
}