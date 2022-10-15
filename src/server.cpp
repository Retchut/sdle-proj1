#include <iostream>
#include <zmq.hpp>
#include <string>
#include <map>
#include <filesystem>

int THREAD_NUM = 2;
std::map<int, std::string> messageMap;

void printUsage (){
    std::string usage = "Usage:\n\t./server\nor\n\t./server put <topic>";
    std::cout << usage << std::endl;
}

int run(){
    while(true){

    }
    return 0;
}

int savePost(std::string folderName, std::string topic, std::string message){

    return 0;
}

int main (int argc, char *argv[]) {
    switch(argc){
        case 1:
            // run server, receiving connections
            std::cout << "Running server" << std::endl;
            return run();
        case 3:
            if(std::string(argv[1]) != "put"){
                printUsage();
                return 1;
            }
            std::cout << "putting message on topic " << std::string(argv[2]) << std::endl;
            return 0;
        default:
            printUsage();
            return 1;
    }

    // int i = 0;
    // while(true){
    //     try{
    //         zmq::message_t request;

    //         std::cout << "waiting for message" << std::endl;
    //         auto res = socket.recv (request, zmq::recv_flags::none); //check flags
    //         std::cout << "Received message: " << request.data() << std::endl;

    //         zmq::message_t reply;
            
    //         memcpy(reply.data(), "reply", 5);
    //         std::cout << "replying " << i << std::endl; 
    //         socket.send(reply, zmq::send_flags::none);
    //         i++;
    //     }
    //     catch(...){
    //         std::cout << "client died" << std::endl;
    //     }
    // }

    return 0;
}
