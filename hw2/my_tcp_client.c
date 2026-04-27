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


int main(void)
{
	const char server_name[] = "127.0.0.1";
	const int  server_port = 29999;
	static int client_socket = 0;

    struct sockaddr_in server_addr = { 0, };
    struct sockaddr_in client_addr = { 0, };
    char ip_addr[16] = { 0, };
    socklen_t addr_size = sizeof(struct sockaddr_in);
    struct hostent* host_entry;
    struct in_addr** address_list;

    // Buffer to store the user input sentence
    char tx_message[1024] = { 0, };
    char rx_buffer[1024] = { 0, };
	int rx_bytes = 0;

    // create a new socket (IPv4, TCP)
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Resolve a domain name to ip address.
    host_entry =  gethostbyname(server_name);
    address_list = (struct in_addr**) host_entry->h_addr_list;
    for (int index = 0; address_list[index] != NULL; ++index) {
        strcpy(ip_addr, inet_ntoa(*address_list[index]));
    }	

	// Put server information into addr struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;				    // IPv4
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);   // server addr
    server_addr.sin_port = htons(server_port);          // server port
	
	// Put client information into addr struct
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;    				// IPv4
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);   	// any interfaces
    client_addr.sin_port = htons(0);				    // random port number given by OS
	
    // connect to the server.
    connect(client_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));

    // Try to extract an address and port number from the socket descriptor.
    getsockname(client_socket, (struct sockaddr*) &client_addr, &addr_size);
    printf("Client is running on port %d\n", ntohs(client_addr.sin_port));

    int option = 0;
    char input_sentence[1024] = {0. };

    while (1){
        printf("<Menu>\n");
        printf("1) convert text to UPPER-case\n");
        printf("2) get server running time\n");
        printf("3) get my IP address and port number\n");
        printf("4) get server request count\n");
        printf("5) exit\n");
        printf("Input option: ");

        scanf("%d", &option);
        getchar();

        if (option == 5){
            printf("Bye bye~\n");
            break;
        }

        memset(tx_message, 0, sizeof(tx_message));
        memset(rx_buffer, 0, sizeof(rx_buffer));

        if (option == 1) {
            printf("Input sentence: ");
            fgets(input_sentence, sizeof(input_sentence), stdin);
            input_sentence[strlen(input_sentence) - 1] = 0;
            fflush(stdin);

            sprintf(tx_message, "%d %s", option, input_sentence);
        } else {
            sprintf(tx_message, "%d", option);
        }

        // send a tx_message to the server.
        write(client_socket, tx_message, strlen(tx_message));

        // receive a reply from the server
        rx_bytes = read(client_socket, rx_buffer, sizeof(rx_buffer));
        if (rx_bytes <= 0) {
            printf("Server disconnected.\n");
            break;
        } 

        rx_buffer[rx_bytes] = 0;

        if (option == 1) {
            printf("Reply from server: %s\n", rx_buffer);
        } else if (option == 2) {
            printf("Reply from server: run time = %s\n", rx_buffer);
        } else if (option == 3) {
            printf("Reply from server: client IP = %s\n", rx_buffer);
        } else if (option == 4) {
            printf("Reply from server: number of requests served = %s\n", rx_buffer);
        }


    }
	close(client_socket);

    return 0;
}