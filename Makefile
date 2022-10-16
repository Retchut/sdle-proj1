all: client server

client:	src/Client.cpp
	g++ src/Client.cpp src/Utils.cpp src/Topic.cpp src/Message.cpp -std=c++17  -lzmq -o build/client 

server:	src/Server.cpp
	g++ src/Server.cpp src/Utils.cpp src/Topic.cpp src/Message.cpp -std=c++17 -lzmq -o build/server

clean:
	rm server client
