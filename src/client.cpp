#include <iostream>
#include <stdexcept>
#include <zmq.hpp>

int clientID;

void printUsage (){
    std::string usage = "Usage:\n\t./client <id>";
    std::cout << usage << std::endl;
}

int main (int argc, char *argv[]) {
    switch(argc){
        case 2:
            try {
                clientID = std::stoi(std::string(argv[1]));
            }
            catch(const std::invalid_argument& e){
                std::cerr << "Invalid argument: " << e.what() << "\n";
                printUsage();
                return 1;
            }
            // run client, receiving instructions
            std::cout << "Running client " << clientID << std::endl;
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
