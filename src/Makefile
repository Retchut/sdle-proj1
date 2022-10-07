all: client server

client:	client.cpp
	g++ client.cpp -lzmq -o client

server:	server.cpp
	g++ server.cpp -lzmq -o server

clean:
	rm server client
