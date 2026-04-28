/*
 * 20213342 박장한
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>

#include <time.h>
#include <signal.h>

#include <sys/select.h>

static volatile sig_atomic_t g_stop = 0;

void handle_sigint(int sig){
    (void)sig;
    g_stop = 1;
}

#define MAX_CLIENTS 100
#define BUF_SIZE 1024

int main(void)
{
    const int server_port = 29999;
    static int server_socket = 0;
    static int client_socket = 0;
    struct sockaddr_in server_addr = { 0, };
    struct sockaddr_in client_addr = { 0, };

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigaction(SIGINT, &sa, NULL);
    
    socklen_t addr_len = sizeof(struct sockaddr_in);

    char rx_buffer[1024] = { 0, };  // request msg from client 
    char tx_buffer[1024] = { 0, };  // response msg to client
	int rx_bytes = 0;
	
    // make a new socket (IPv4, TCP)
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // put server information into a socket.
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;					// IPv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	// any ip address (interface)
    server_addr.sin_port = htons(server_port);			// server port
	
    // bind the socket with the server address.
    bind(server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));

    // 시간 측정 시작 & 요청 Count 초기화
    time_t start_time = time(NULL);
    int request_count = 0;

    printf("Server is ready to receive on port %d\n", ntohs(server_addr.sin_port));

    // ready to listen to clients.
    listen(server_socket, 10);


    // Loop
    int client_sockets[MAX_CLIENTS];
    int client_ids[MAX_CLIENTS];
    int client_count = 0;
    int next_client_id = 1;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = -1;
        client_ids[i] = -1;
    }

    fd_set read_fds;
    int max_fd = 0;
    int activity = 0;
    struct timeval timeout;

    while (!g_stop) {
        // 서버 소켓으로 클라이언트가 요청을 보내는지 감시
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        max_fd = server_socket;

        // 클라이언트가 존재하는 소켓의 경우 메시지가 오는지 감시
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1) {
                FD_SET(client_sockets[i], &read_fds);

                if (client_sockets[i] > max_fd) {
                    max_fd = client_sockets[i];
                }
            }
        }

        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

        if (activity < 0) {
            if (g_stop) {
                break;
            }
            perror("select");
            continue;
        }

        if (activity == 0) {
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            char time_str[16];

            strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);

            printf("[%s] Number of connected client = %d\n",
                time_str, client_count);
            continue;
        }

        if (FD_ISSET(server_socket, &read_fds)) {
            client_socket = accept(server_socket,
                                (struct sockaddr *)&client_addr,
                                &addr_len);

            if (client_socket < 0) {
                if (g_stop) {
                    break;
                }
                perror("accept");
                continue;
            }

            int empty_index = -1;

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == -1) {
                    empty_index = i;
                    break;
                }
            }

            if (empty_index == -1) {
                // 소켓이 다 차면 새로 연결된 클라이언트 닫기
                close(client_socket);
            } else {
                client_sockets[empty_index] = client_socket;
                client_ids[empty_index] = next_client_id++;
                client_count++;

                time_t now = time(NULL);
                struct tm *tm_info = localtime(&now);
                char time_str[16];

                strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);

                printf("[%s] Client %d connected. Number of connected clients = %d\n",
                    time_str,
                    client_ids[empty_index],
                    client_count);
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1 &&
                // 클라이언트 소켓 배열 통해서 신호가 들어온 경우 처리
                FD_ISSET(client_sockets[i], &read_fds)) {

                memset(rx_buffer, 0, sizeof(rx_buffer));
                memset(tx_buffer, 0, sizeof(tx_buffer));

                rx_bytes = read(client_sockets[i], rx_buffer, sizeof(rx_buffer) - 1);

                if (rx_bytes <= 0) {
                    time_t now = time(NULL);
                    struct tm *tm_info = localtime(&now);
                    char time_str[16];

                    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);

                    printf("[%s] Client %d disconnected. Number of connected clients = %d\n",
                        time_str,
                        client_ids[i],
                        client_count - 1);

                    close(client_sockets[i]);
                    client_sockets[i] = -1;
                    client_ids[i] = -1;
                    client_count--;

                    continue;
                }

                rx_buffer[rx_bytes] = '\0';

                int option = 0;
                char message[1024] = {0};

                sscanf(rx_buffer, "%d %[^\n]", &option, message);

                request_count++;

                if (option == 1) {
                    size_t len = strlen(message);

                    for (size_t index = 0; index < len; index++) {
                        tx_buffer[index] = toupper((unsigned char)message[index]);
                    }

                    tx_buffer[len] = '\0';
                } else if (option == 2) {
                    time_t now = time(NULL);
                    int elapsed = (int)difftime(now, start_time);

                    int hours = elapsed / 3600;
                    int minutes = (elapsed % 3600) / 60;
                    int seconds = elapsed % 60;

                    snprintf(tx_buffer, sizeof(tx_buffer),
                            "%02d:%02d:%02d",
                            hours, minutes, seconds);
                } else if (option == 3) {
                    struct sockaddr_in peer_addr;
                    socklen_t peer_len = sizeof(peer_addr);

                    getpeername(client_sockets[i],
                                (struct sockaddr *)&peer_addr,
                                &peer_len);

                    snprintf(tx_buffer, sizeof(tx_buffer),
                            "%s, port = %d",
                            inet_ntoa(peer_addr.sin_addr),
                            ntohs(peer_addr.sin_port));
                } else if (option == 4) {
                    snprintf(tx_buffer, sizeof(tx_buffer),
                            "%d", request_count);
                } else {
                    snprintf(tx_buffer, sizeof(tx_buffer),
                            "Invalid option");
                }

                if (write(client_sockets[i], tx_buffer, strlen(tx_buffer)) < 0) {
                    perror("write");
                }
            }
        }
    }




    // 모든 소켓 안전하게 종료
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != -1) {
            close(client_sockets[i]);
        }
    }

    close(server_socket);	
    printf("Bye bye server~ \n");

    return 0;
}