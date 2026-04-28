CC = gcc
CFLAGS = -Wall -Wextra -O2

TARGETS = multi_tcp_server multi_tcp_client

all: $(TARGETS)

multi_tcp_server: multi_tcp_server.c
	$(CC) $(CFLAGS) -o multi_tcp_server multi_tcp_server.c

multi_tcp_client: multi_tcp_client.c
	$(CC) $(CFLAGS) -o multi_tcp_client multi_tcp_client.c

clean:
	rm -f $(TARGETS)