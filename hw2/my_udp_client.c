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
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);

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

	printf("Input sentence: ");
    fgets(tx_message, 1024, stdin);
    tx_message[strlen(tx_message) - 1] = 0;
    fflush(stdin);

    // send a tx_message to the server.
    sendto(client_socket, tx_message, strlen(tx_message), 0,
          (struct sockaddr*) &server_addr, sizeof(server_addr));

    // receive a reply from the server
    rx_bytes = recvfrom(client_socket, rx_buffer, sizeof(rx_buffer), 0,
                             (struct sockaddr*) &server_addr, &addr_size);
    rx_buffer[rx_bytes] = 0;

    printf("\nReply from server: %s\n", rx_buffer);
	
	close(client_socket);

    return 0;
}
