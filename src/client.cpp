#include <iostream>
#include <zmq.hpp>

void printUsage (){
    std::string usage = "Usage:\n\t./client\nor\n\t./client put <topic>";
    std::cout << usage << std::endl;
}
int main (int argc, char *argv[]) {
    switch(argc){
        case 1:
            // run server, receiving connections
            std::cout << "Running client eventually" << std::endl;
            return 0;
        case 3:
            if(std::string(argv[1]) != "get"){
                printUsage();
                return 1;
            }
            std::cout << "getting message from topic " << std::string(argv[2]) << std::endl;
            return 0;
        default:
            printUsage();
            return 1;
    }

    // int i = 0;
    // while(true){
    //     zmq::message_t request(4);

    //     memcpy(request.data(), "test", 4);
    //     std::cout << "sent message " << i << std::endl;
    //     socket.send (request, zmq::send_flags::none);

    //     //Get a reply
    //     zmq::message_t reply;

    //     std::cout << "waiting for reply" << std::endl;
    //     socket.recv (reply, zmq::recv_flags::none);
    //     std::cout << reply.data() << " " << i << std::endl;
    //     i++;
    // }

    return 0;
}
