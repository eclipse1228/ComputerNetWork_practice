#include "../Common.h"
#include <string.h>
#include <stdlib.h>

#define SERVERPORT 9000
#define BUFSIZE    512

// 숫자를 문자로 바꾸는 함수
void itoa(int num, char *str, int radix) {
        int tmp = num;
        int cnt = 0;

        while(tmp != 0) {
                tmp /= 10;
                cnt++;
        }

        str[cnt] = '\0';
        do {
                cnt--;
                str[cnt] = (char)(num%10 + 48);
                num /= 10;
        } while (num != 0);
}

// 월별 일수를 반환하는 함수
int days_of_month(int year, int month) {
        int month_of_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        month_of_days[1] += is_leaf_year(year);
        return month_of_days[month - 1];
}

// 윤년인지 아닌지 판별하는 함수
int is_leaf_year(int year) {
        if((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
                return 1;
        }
        return 0;
}

// 1년 1월 1일부터 입력받은 년, 월, 일까지의 총 일수를 반환하는 함수
int get_day(int year, int month) {
        int total_days = 0;
        for(int i = 1; i < year; i++) {
                total_days += 365 + is_leaf_year(i);
        }
        for(int i = 1; i < month; i++) {
                total_days += days_of_month(year, i);
        }
        return total_days;
}
// 달력을 출력하는 함수, buf 변수에 달력의 정보를 문자열의 형태로 저장.
void print_calendar(int year, int month, char *buf) {
        // 1년 1월 1일부터 지금까지 총 일수
        int total_days = get_day(year, month);
        // 입력받은 년 월의 1일의 요일
        int first_day = (total_days + 1) % 7;
        // 이번 달 총 일수
        int days = days_of_month(year, month);
        // 첫번째 줄
        printf("SUN MON TUE WED THU FRI SAT\n");
        strcat(buf, "SUN MON TUE WED THU FRI SAT\n");
        for(int i = 0; i < first_day; i++) {
                printf("    ");
                strcat(buf, "    ");
        }
        // 달력의 요일을 표시
        for(int i = 1; i <= days; i++) {
                char dayBuf[3];
                printf("%4d", i);
                if(i >= 10) {
                        strcat(buf, "  ");
                }
                else {
                        strcat(buf, "   ");
                }
                itoa(i, dayBuf, 3);
                strcat(buf, dayBuf);
                if((first_day + i) % 7 == 0) {
                        printf("\n");
                        strcat(buf, "\n");
                }
        }
        printf("\n");
        strcat(buf, "\n");
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
                        retval = recv(client_sock, buf, BUFSIZE, 0);
                        if (retval == SOCKET_ERROR) {
                                err_display("recv()");
                                break;
                        }
                        else if (retval == 0)
                                break;

                        // 입력받은 데이터 전처리하기
                        int year = 0, month = 0, dotPos = 4;
                        if(buf[dotPos] != '.') {
                                printf("Wrong input format. Please input yyyy. mm format\n");
                                continue;
                        }
                        // 년도 값을 숫자로 저장
                        for(int i = 0; i < dotPos; i++) {
                                year += (int)buf[i] - 48;
                                if(i == dotPos - 1) break;
                                year *= 10;
                        }
                        // 월 값을 숫자로 저장
                        month = ((int)buf[dotPos+2] - 48) * 10 + buf[dotPos+3] - 48;
                        // 달력 출력 및 buf에 달력 문자열 저장
                        buf[0] = '\0';
                        printf("%d년 %d월 달력 출력\n", year, month);
                        print_calendar(year, month, buf);

                        // 데이터 보내기
                        retval = send(client_sock, buf, BUFSIZE, 0);
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