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
        {
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

            // Testing
            zmq::context_t context(1);
            zmq::socket_t socket (context, zmq::socket_type::req);
            socket.connect("tcp://127.0.0.1:5555");

            int i = 0;
            while(true){
                //Read from stdin
                std::string line;
                std::getline(std::cin, line);

                zmq::message_t request(line.length());

                //Send
                memcpy(request.data(), line.c_str(), line.length());
                socket.send (request, zmq::send_flags::none);
                std::cout << "sent message: " << line.c_str() << std::endl;

                //Get a reply
                zmq::message_t reply;
                std::cout << "waiting for reply" << std::endl;
                auto size = socket.recv (reply, zmq::recv_flags::none);
                char * reply_c_str = (char *) reply.data();
                reply_c_str[size.value()] = '\0';
                std::cout << "Reply: " << reply_c_str << std::endl;
                i++;
            }
            // END-Testing
        }
            return 0;
        default:
            printUsage();
            return 1;
    }

    return 0;
}
