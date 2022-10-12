#include <iostream>
#include <zmq.hpp>
#include <string>

int THREAD_NUM = 2;

int main () {
    //  Prepare our context and socket
    zmq::context_t context (THREAD_NUM);
    zmq::socket_t socket(context, zmq::socket_type::rep);
    socket.bind("tcp://127.0.0.1:5555");

    int i = 0;
    while(true){
        try{
            zmq::message_t request;

            std::cout << "waiting for message" << std::endl;
            auto res = socket.recv (request, zmq::recv_flags::none); //check flags
            std::cout << "Received message: " << request.data() << std::endl;

            zmq::message_t reply;
            
            memcpy(reply.data(), "reply", 5);
            std::cout << "replying " << i << std::endl; 
            socket.send(reply, zmq::send_flags::none);
            i++;
        }
        catch(...){
            std::cout << "client died" << std::endl;
            return 1;
        }
    }

    return 0;
}
