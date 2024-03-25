#include "Common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVERPORT 9000
#define BUFSIZE    3000
int dayInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


// 윤년 체크하기 해서 더하기 윤년이면 1
int yearCheck(int year) {
	if (year % 4 != 0) return 0;
	if (year % 100 != 0) return 1;
	if (year % 400 == 0) return 1;
	return 0;
}

// 1970년 이후로 인풋 대비 알마나 지났는지
int getTotalDay(int year, int month) { 
	int totalDay = 0;
	for (int i = 1970; i < year; i++) {
		totalDay += 365 + yearCheck(i);
	} 

	for (int j = 0; j < month-1; j++) {
		totalDay += dayInMonth[j];
		if (j == 1 && yearCheck(year)) totalDay += 1; // 2월 예외처리 
	}
	return totalDay;
}


// 출력하기
void printCalender(int year, int month, char *calenderStr) { 
    char calender[3000];
    strcpy(calenderStr, " SUN MON TUE WED THU FRI SAT\n");

	int totalDay = getTotalDay(year, month);
	int startDay = (totalDay + 4) % 7; // +4 은1970년 1월 1일이 목요일이라서 일요일에 맞춤.

	for (int i = 0; i < startDay; i++) {
		strcat(calenderStr, "    ");
	}

	int monthDay = dayInMonth[month - 1];
	if (month == 2 && yearCheck(year) == 1) monthDay += 1; 

	for (int j = 1; j <= monthDay; j++) {
        sprintf(calender, " %3d",j);
        strcat(calenderStr,calender);
		if ((startDay + j) % 7 == 0) strcat(calenderStr,"\n");
	}
	strcat(calenderStr,"\n"); // 마지막에 NULL
}

int main(int argc, char *argv[])
{
        int retval;

        // 소켓 생성
        SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_sock == INVALID_SOCKET) err_quit("socket()");

        // bind()
        struct sockaddr_in serveraddr;
        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serveraddr.sin_port = htons(SERVERPORT);
        retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        if (retval == SOCKET_ERROR) err_quit("bind()");

        // listen()
        retval = listen(listen_sock, SOMAXCONN);
        if (retval == SOCKET_ERROR) err_quit("listen()");

        // 데이터 통신에 사용할 변수
        SOCKET client_sock;
        struct sockaddr_in clientaddr;
        socklen_t addrlen;
        char buf[BUFSIZE + 1];
        char calenderStr[BUFSIZE];
        int yearToken = 0; // 년,월
        int monthToken = 0;

        while (1) {
               // accept()
                addrlen = sizeof(clientaddr);
                client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
                if (client_sock == INVALID_SOCKET) {
                        err_display("accept()");
                        break;
                }

                // 접속한 클라이언트 정보 출력
                char addr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
                printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
                        addr, ntohs(clientaddr.sin_port));

                // 클라이언트와 데이터 통신
                while (1) {
                        // 데이터 받기
                        retval = recv(client_sock, buf, BUFSIZE, 0); // int 형임 retval는 
                        if (retval == SOCKET_ERROR) { // 에러 
                                err_display("recv()");
                                break;
                        }
                        else if (retval == 0)
                                break;

                        // 받은 데이터 출력 
                        buf[retval] = '\0'; // 끝에 NULL 추가. 클라이언트가 NULL을 기준으로 데이터를 수신할 수 있음.
                        printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);
                        
                        // 받은 2020.03을 yearToken: 2020과 monthToken:03으로 나누기
                 
                        char *token = strtok(buf,".");
                        int count = 0;
                        while(token != NULL) { 
                            if(count == 0) yearToken= atoi(token);
                            if(count == 1) monthToken = atoi(token);
                            token = strtok(NULL,".");
                            count ++;                        
                        }
                        printf("Year: %d, Month: %d\n", yearToken, monthToken);

                        
                        // 서버가 보내는것 출력.
                        printCalender(yearToken,monthToken,calenderStr);

                        // 보내기 전에 데이터 사이즈 먼저 보내기 ( 동적으로 데이터 사이즈가 변함(28~31일))
                        int dataSize = strlen(calenderStr);
                        int netDataSize = htonl(dataSize);               // network host to networok (network는 빅엔디안 방식임 따라서 리틀->빅 변환 )
                        send(client_sock, &netDataSize,  sizeof(netDataSize),0);


                        // 데이터 보내기 
                        retval = send(client_sock, calenderStr,strlen(calenderStr), 0);
                        if (retval == SOCKET_ERROR) {
                                err_display("send()");
                                break;
                        }
                }

                // 소켓 닫기
                close(client_sock);
                printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
                        addr, ntohs(clientaddr.sin_port));
        }

        // 소켓 닫기
        close(listen_sock);
        return 0;
}
