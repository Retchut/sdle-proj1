#include <iostream>
#include <zmq.hpp>

int main () {
    //  Prepare our context and socket
    zmq::context_t context(1);
    zmq::socket_t socket (context, zmq::socket_type::req);
    socket.connect("tcp://127.0.0.1:5555");

    zmq::message_t request(4);
    memcpy(request.data(), "test", 4);
    std::cout << "sent message" << std::endl;
    socket.send (request, zmq::send_flags::none);

    return 0;
}
