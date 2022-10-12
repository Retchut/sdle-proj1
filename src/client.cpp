#include <iostream>
#include <zmq.hpp>

int main () {
    //  Prepare our context and socket
    zmq::context_t context(1);
    zmq::socket_t socket (context, zmq::socket_type::req);
    socket.connect("tcp://127.0.0.1:5555");

    int i = 0;
    while(true){
        zmq::message_t request(4);

        memcpy(request.data(), "test", 4);
        std::cout << "sent message " << i << std::endl;
        socket.send (request, zmq::send_flags::none);

        //Get a reply
        zmq::message_t reply;

        std::cout << "waiting for reply" << std::endl;
        socket.recv (reply, zmq::recv_flags::none);
        std::cout << reply.data() << " " << i << std::endl;
        i++;
    }

    return 0;
}
