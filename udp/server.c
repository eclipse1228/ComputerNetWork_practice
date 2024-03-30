#include "Common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>

#define SERVERPORT 9000
#define BUFSIZE    3071 // novel.txt 바이트 계산 결과

int main(int argc, char *argv[])
{
	int retval;

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;	
	memset(&serveraddr, 0, sizeof(serveraddr)); // 서버 주소를 0으로 다 초기화합니다. (쓰레기값으로 인한 오류를 막는다.)
	serveraddr.sin_family = AF_INET;            // 인터넷 통신 체계 
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);   // 
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// 데이터 통신에 사용할 변수
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char buf[BUFSIZE + 1];

	// 클라이언트와 데이터 통신
	while (1) {
		// 버퍼 0으로 초기화하기 (제대로 요청해도 가끔 이상하게 요청했다고 해서)
		memset(buf, 0, BUFSIZE + 1);

		// 데이터 받기
		addrlen = sizeof(clientaddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(struct sockaddr *)&clientaddr, &addrlen);

		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			break;
		}
		
		buf[retval] = '\0';
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[UDP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);
		printf("The server received a request from a client\n");
		

		if(strcmp(buf,"novel.txt") == 0){ // 받은 소켓의 내용 즉, 파일이름이 novel.txt 이면 출력, 아니면 안된다고 출력 
			// 파일 열기
			int fileOpen = open("novel.txt", O_RDONLY);

			// 파일 전송하기 
			int byte;
			int readResult;
			while((readResult = read(fileOpen,buf, BUFSIZE)) > 0) {
				byte = sendto(sock,buf,readResult,0,(struct sockaddr *)&clientaddr, addrlen);
				if (retval == SOCKET_ERROR) {
					err_display("sendto()");
					break;
				}
			}
			
			printf("The server sent “novel.txt” to the client\n");
			// 파일 닫기
		    close(fileOpen);
		}

		else {
			printf("클라이언트가 novel이 아닌 다른 요청을 보냈습니다.\n");
			char *errMsg = "요청한 파일은 없는 파일이다.";
			sendto(sock,errMsg,strlen(errMsg),0,(struct sockaddr *)&clientaddr, addrlen);
		}

		// 받은 데이터 출력

	}

	// 소켓 닫기

	close(sock); 
	return 0;
}
