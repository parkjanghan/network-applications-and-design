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

void handle_sigint(int sig)
{
    (void)sig;
    g_stop = 1;
}

int main(void)
{
    const int server_port = 29999;
    static int server_socket = 0;

    struct sockaddr_in server_addr = {0, };
    struct sockaddr_in client_addr = {0, };
    socklen_t addr_len = sizeof(client_addr);

    char rx_buffer[1024] = {0, };
    char tx_buffer[1024] = {0, };
    int rx_bytes = 0;

    time_t start_time = time(NULL);
    int request_count = 0;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigaction(SIGINT, &sa, NULL);

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("Server is ready to receive on port %d\n", ntohs(server_addr.sin_port));

    while (!g_stop) {
        memset(rx_buffer, 0, sizeof(rx_buffer));
        memset(tx_buffer, 0, sizeof(tx_buffer));

        addr_len = sizeof(client_addr);

        rx_bytes = recvfrom(server_socket, rx_buffer, 1023, 0,
                            (struct sockaddr *)&client_addr, &addr_len);

        if (rx_bytes <= 0) {
            if (g_stop) {
                break;
            }
            continue;
        }

        rx_buffer[rx_bytes] = '\0';

        printf("Request msg from ('%s', %d): %s\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port),
               rx_buffer);

        int option = 0;
        char message[1024] = {0, };

        sscanf(rx_buffer, "%d %[^\n]", &option, message);

        request_count++;

        if (option == 1) {
            size_t len = strlen(message);

            for (size_t index = 0; index < len; ++index) {
                tx_buffer[index] = (char)toupper((unsigned char)message[index]);
            }

            tx_buffer[len] = '\0';
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

        sendto(server_socket, tx_buffer, strlen(tx_buffer), 0,
               (struct sockaddr *)&client_addr, addr_len);
    }

    close(server_socket);
    printf("Bye bye~\n");

    return 0;
}