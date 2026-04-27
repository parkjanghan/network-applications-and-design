CC = gcc
CFLAGS = -Wall -Wextra -O2

TARGETS = my_tcp_client my_tcp_server my_udp_client my_udp_server

all: $(TARGETS)

my_tcp_client: my_tcp_client.c
	$(CC) $(CFLAGS) -o my_tcp_client my_tcp_client.c

my_tcp_server: my_tcp_server.c
	$(CC) $(CFLAGS) -o my_tcp_server my_tcp_server.c

my_udp_client: my_udp_client.c
	$(CC) $(CFLAGS) -o my_udp_client my_udp_client.c

my_udp_server: my_udp_server.c
	$(CC) $(CFLAGS) -o my_udp_server my_udp_server.c

clean:
	rm -f $(TARGETS)