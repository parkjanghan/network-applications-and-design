/*
 * 20213342 박장한
 */

#include <stdio.h>
#include <string.h>
#include <netdb.h>
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
    const char server_name[] = "127.0.0.1";
    const int server_port = 29999;
    static int client_socket = 0;

    struct sockaddr_in server_addr = {0, };
    struct sockaddr_in client_addr = {0, };
    struct sockaddr_in from_addr = {0, };
    char ip_addr[16] = {0, };
    socklen_t addr_size = sizeof(struct sockaddr_in);
    socklen_t from_len = sizeof(from_addr);
    struct hostent *host_entry;
    struct in_addr **address_list;

    char tx_message[1024] = {0, };
    char rx_buffer[1024] = {0, };
    char input_sentence[1024] = {0, };
    int rx_bytes = 0;
    int option = 0;

    struct timespec send_time;
    struct timespec recv_time;
    double rtt_ms = 0.0;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigaction(SIGINT, &sa, NULL);

    client_socket = socket(AF_INET, SOCK_DGRAM, 0);

    host_entry = gethostbyname(server_name);
    address_list = (struct in_addr **)host_entry->h_addr_list;

    for (size_t index = 0; address_list[index] != NULL; ++index) {
        strcpy(ip_addr, inet_ntoa(*address_list[index]));
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    server_addr.sin_port = htons(server_port);

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    client_addr.sin_port = htons(0);

    bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr));

    getsockname(client_socket, (struct sockaddr *)&client_addr, &addr_size);
    printf("Client is running on port %d\n", ntohs(client_addr.sin_port));

    while (!g_stop) {
        printf("<Menu>\n");
        printf("1) convert text to UPPER-case\n");
        printf("2) get server running time\n");
        printf("3) get my IP address and port number\n");
        printf("4) get server request count\n");
        printf("5) exit\n");
        printf("Input option: ");

        if (scanf("%d", &option) != 1) {
            break;
        }

        if (g_stop) {
            break;
        }

        getchar();

        if (option == 5) {
            break;
        }

        memset(tx_message, 0, sizeof(tx_message));
        memset(rx_buffer, 0, sizeof(rx_buffer));

        if (option == 1) {
            printf("Input sentence: ");

            if (fgets(input_sentence, sizeof(input_sentence), stdin) == NULL) {
                break;
            }

            if (g_stop) {
                break;
            }

            input_sentence[strcspn(input_sentence, "\n")] = '\0';
            snprintf(tx_message, sizeof(tx_message), "%d %s", option, input_sentence);
        }
        else {
            snprintf(tx_message, sizeof(tx_message), "%d", option);
        }

        clock_gettime(CLOCK_MONOTONIC, &send_time);

        sendto(client_socket, tx_message, strlen(tx_message), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr));

        from_len = sizeof(from_addr);
        rx_bytes = recvfrom(client_socket, rx_buffer, 1023, 0,
                            (struct sockaddr *)&from_addr, &from_len);

        clock_gettime(CLOCK_MONOTONIC, &recv_time);

        if (rx_bytes <= 0) {
            break;
        }

        rx_buffer[rx_bytes] = '\0';

        if (option == 1) {
            printf("Reply from server: %s\n", rx_buffer);
        }
        else if (option == 2) {
            printf("Reply from server: run time = %s\n", rx_buffer);
        }
        else if (option == 3) {
            printf("Reply from server: client IP = %s\n", rx_buffer);
        }
        else if (option == 4) {
            printf("Reply from server: number of requests served = %s\n", rx_buffer);
        }

        rtt_ms =
            (recv_time.tv_sec - send_time.tv_sec) * 1000.0 +
            (recv_time.tv_nsec - send_time.tv_nsec) / 1000000.0;

        printf("RTT = %.3f ms\n", rtt_ms);
    }

    close(client_socket);
    printf("Bye bye~\n");

    return 0;
}