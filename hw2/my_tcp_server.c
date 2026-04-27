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

static volatile sig_atomic_t g_stop = 0;

void handle_sigint(int sig){
    (void)sig;
    g_stop = 1;
}

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
    listen(server_socket, 1);

    // Loop
    while (!g_stop)
    {
		// accept clients and build connections.
		client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &addr_len);

        if (client_socket < 0) {
            if (g_stop) break;
            continue;
        }

        while(!g_stop){
            memset(rx_buffer, 0, sizeof(rx_buffer));
            memset(tx_buffer, 0, sizeof(tx_buffer));

            // receive a request message from the client.
            rx_bytes = read(client_socket, rx_buffer, 1023);

            // Does it have any error?
            if (rx_bytes <= 0) {
                break;
            }
            rx_buffer[rx_bytes] = 0;

            // 디버깅
            printf("Request msg from ('%s', %d): %s\n",
                    inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), rx_buffer);

            // rx_bytes에 들어온 client의 입력값 수정
            int option = 0;
            char message[1024] = { 0, };

            memset(tx_buffer, 0, sizeof(tx_buffer));

            sscanf(rx_buffer, "%d %[^\n]", &option, message);

            request_count++;

            if (option == 1) {
                size_t len = strlen(rx_buffer);
                for (size_t index = 0; index < len; ++index) {
                    tx_buffer[index] = toupper(message[index]);
                }
                tx_buffer[strlen(message)] = 0;
            }
            else if (option == 2) {
                time_t now = time(NULL);
                int elapsed = (int)difftime(now, start_time);

                int hours = elapsed / 3600;
                int minutes = (elapsed % 3600) / 60;
                int seconds = elapsed % 60;

                sprintf(tx_buffer, "%02d:%02d:%02d", hours, minutes, seconds);
            }
            else if (option == 3) {
                sprintf(tx_buffer, "%s, port = %d",
                        inet_ntoa(client_addr.sin_addr),
                        ntohs(client_addr.sin_port));
            }
            else if (option == 4) {
                sprintf(tx_buffer, "%d", request_count);
            }
            else {
                sprintf(tx_buffer, "Invalid option");
            }
            
            // Send the message.
            write(client_socket, tx_buffer, strlen(tx_buffer));
        }

        close(client_socket);
    }

    close(server_socket);	
    printf("Bye bye~ \n");

    return 0;
}