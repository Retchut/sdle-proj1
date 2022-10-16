#include <iostream>
#include <stdexcept>
#include <zmq.hpp>
#include <map>
#include <string>

int clientID;

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
            break;
        default:
            printUsage();
            return 1;
    }

    std::map<std::string, int> subscribedTopics;

    subscribeTopic(subscribedTopics, "praxe");
    subscribeTopic(subscribedTopics, "tuna");
    printSubscribedTopics(subscribedTopics);

    unsubscribeTopic(subscribedTopics, "praxe");
    printSubscribedTopics(subscribedTopics);

    changeLastMessageReceivedFromTopic(subscribedTopics, "tuna", 3);
    printSubscribedTopics(subscribedTopics);

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
