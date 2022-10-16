#include <iostream>
#include <stdexcept>
#include <string>
#include <map>
#include <zmq.hpp>

#include "Utils.h"

int clientID;
std::string entityName;

void printUsage (){
    std::string usage = "Usage:\n\t./client <id>";
    std::cout << usage << std::endl;
}

void printSubscribedTopics (std::map<std::string, int> &subscribedTopics) {
    for (auto itr = subscribedTopics.begin(); itr != subscribedTopics.end(); ++itr) {
        std::cout << itr->first << '\t' << itr->second << '\n';
    }
}

void subscribeTopic (std::map<std::string, int> &subscribedTopics, std::string topic) {
    subscribedTopics.insert(std::pair<std::string, int>(topic, -1));
}

int unsubscribeTopic (std::map<std::string, int> &subscribedTopics, std::string topic) {
    return subscribedTopics.erase(topic);
}

int changeLastMessageReceivedFromTopic (std::map<std::string, int> &subscribedTopics, std::string topic, int messageID) {
    if(subscribedTopics.find(topic) != subscribedTopics.end()) {
        subscribedTopics.at(topic) = messageID;
        return 1;
    }
    return 0;
}

void testClient(){
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

void runClient(){
    std::map<std::string, int> subscribedTopics;

    subscribeTopic(subscribedTopics, "praxe");
    subscribeTopic(subscribedTopics, "tuna");
    printSubscribedTopics(subscribedTopics);

    unsubscribeTopic(subscribedTopics, "praxe");
    printSubscribedTopics(subscribedTopics);

    changeLastMessageReceivedFromTopic(subscribedTopics, "tuna", 3);
    printSubscribedTopics(subscribedTopics);
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
            entityName = "client" + std::to_string(clientID);
            setupStorage(entityName);

            std::cout << "Running client " << clientID << std::endl;
            // testClient();
            runClient();
        }
            return 0;
        default:
            printUsage();
            return 1;
    }

    return 0;
}
