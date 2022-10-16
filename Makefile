all: client server

client:	client.cpp
	g++ client.cpp utils.cpp -lzmq -o client

server:	server.cpp
	g++ server.cpp utils.cpp -lzmq -o server

clean:
	rm server client
