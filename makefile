all: server client
clean:
	rm -f server client

server: server.c
	gcc server.c -lwsock32 -lws2_32 -o server

client: client.c
	gcc client.c -lwsock32 -lws2_32 -o client
