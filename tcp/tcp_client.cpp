#include "Common.h"

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    100

int main(int argc, char *argv[])
{
        int retval;

        // 명령행 인수가 있으면 IP 주소로 사용
        if (argc > 1) SERVERIP = argv[1];

        // 소켓 생성
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) err_quit("socket()");

        // connect()
        struct sockaddr_in serveraddr;
        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
        serveraddr.sin_port = htons(SERVERPORT);
        retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        if (retval == SOCKET_ERROR) err_quit("connect()");

        // 데이터 통신에 사용할 변수
        char buf[BUFSIZE + 1];
        int len;

        // 서버와 데이터 통신
        while (1) {
                // 데이터 입력
                printf("\n[보낼 데이터] ");
                if (fgets(buf, BUFSIZE + 1, stdin) == NULL) break;

                // '\n' 문자 제거
                len = (int)strlen(buf);
                if (buf[len - 1] == '\n')
                        buf[len - 1] = '\0';
                if (strlen(buf) == 0)
                        break;
                                        // 데이터 보내기
                retval = send(sock, buf, (int)strlen(buf), 0);
                if (retval == SOCKET_ERROR) {
                        err_display("send()");
                        break;
                }

                printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);
                // 받기 전에 데이터 크기를 수신 받습니다. 
                int dataSize = 0 ;
                recv(sock,&dataSize,sizeof(dataSize),0);
                dataSize = ntohl(dataSize); 

                // 데이터 크기에 따른 동적할당 필요. (BUFSIZE 사용하지 않음.)
                char* dataBuf = malloc(dataSize +1 ); // 수신을 위한 버퍼 
                int totalReceived = 0;
                while(totalReceived < dataSize) {
                    int recevied = recv(sock,dataBuf + totalReceived, dataSize -totalReceived,0); // 2번째 인자는 포인터임, 3번째 인자는 데이터 크기임(안 빼주면 오버플로우 맞음)
                    if(recevied <=0 ) break;
                    totalReceived += recevied;
                }

                dataBuf[dataSize] = '\0'; // 끝에 NULL 처리
                printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", totalReceived);
                printf("[받은 데이터] \n%s\n", dataBuf);
                                
                free(dataBuf); // 할당 해제
                }
        
                // retval = recv(sock, buf, BUFSIZE, MSG_WAITALL); // retval 하면, 내가 보낸 2020.03. 만큼만 받음. 따라서 받을때는 크게 받아야함. 
                // 이거 문제가 뭐냐면.. 달력이 총 얼마나 출력되어야하는지가 문제다. 즉 상대방이 얼마나 줄지 알아야한다.
                // 그런데. 28일,30일,31일도 있어서.. 어렵다. 그러면.. 마지막 28일이라면,. 그만큼 29 30일만큼 공백을 추가해서 넘겨야한다.
                // 그러지 않으면, 아니지. 31일까지만 받으면 되지. (X) -> 만약 31일치 받으려고 요청했는데, 28일이면 서버에서 줄때까지 기다리기 때문에 무한 대기에 빠지게됨.
                // 혹은 버퍼 사이즈 매개변수로 입력하지 않고 send() 를 보내면 , 주는대로 받는것인가? (X) : 안 됨. 

                // 그럼 사이즈를 먼저 전달해서 그것을 BUFSIZE로 받자.(X) : 안 됨 
                // NULL일때까지 받자. (X) : 에초에 요청할 때 BUFSIZE로 요청해서, 28일이 올지 31일이 올지 모름. -> 120~130비트 사이에 랜덤으로 옴. 안됨
                // 그냥 먼저 데이터 크기를 받고, 그대로 요청하도록하자 (O) : 됨.
                // 얼만큼 올지 모른다 -> 동적 할당 사용 -> 되나? -> 됨. (O)
        
        // 소켓 닫기
        close(sock);
        return 0;
}
