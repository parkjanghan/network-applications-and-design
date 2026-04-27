#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>


int main(void)
{
    const int server_port = 29999;
    static int server_socket = 0;
    static int client_socket = 0;
    struct sockaddr_in server_addr = { 0, };
    struct sockaddr_in client_addr = { 0, };
    
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

    printf("Server is ready to receive on port %d\n", ntohs(server_addr.sin_port));

    // ready to listen to clients.
    listen(server_socket, 1);

    // Loop
    while (1)
    {
		// accept clients and build connections.
		client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &addr_len);

        // receive a request message from the client.
		rx_bytes = read(client_socket, rx_buffer, 1024);

		// Does it have any error?
		if (rx_bytes < 0) {
			close(client_socket);
			break;
		}
		rx_buffer[rx_bytes] = 0;

        printf("Request msg from ('%s', %d): %s\n",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), rx_buffer);

        // Make a response message, then send to the client.
		for (int index = 0; index < strlen(rx_buffer); ++index) {
			tx_buffer[index] = toupper(rx_buffer[index]);
		}
        tx_buffer[strlen(rx_buffer)] = 0;
		
		// Send the message.
		write(client_socket, tx_buffer, strlen(tx_buffer));

        close(client_socket);
    }

    close(server_socket);	

    return 0;
}