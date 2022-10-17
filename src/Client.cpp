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

int changeLastMessageID (std::map<std::string, int> &subscribedTopics, std::string topic, int messageID) {
    if(subscribedTopics.find(topic) != subscribedTopics.end()) {
        subscribedTopics.at(topic) = messageID;
        return 1;
    }
    return 0;
}

int getLastMessageIDFromTopic (std::map<std::string, int> &subscribedTopics, std::string topic) {
    if(subscribedTopics.find(topic) != subscribedTopics.end()) {
        return subscribedTopics.at(topic);
    }
    return -1;
}

void testClientCommunication(int clientID){
    // Testing
    zmq::context_t context(1);
    zmq::socket_t socket (context, zmq::socket_type::req);
    socket.connect("tcp://127.0.0.1:5555");

    int i = 0;
    std::string topic_name;
    if (clientID == 1)
        topic_name = "2";
    else topic_name = "1";

    while(true){
        //Read from stdin
        std::string line;
        if (i > 2) i = 0;
        if (i == 0)
            line = "SUB " + std::to_string(clientID) + " " + topic_name;
        else if (i == 1)
            line = "PUT " + std::to_string(clientID) + " " + std::to_string(clientID) + " mensagem_catita" ;
        else if (i == 2)
            line = "GET " + std::to_string(clientID) + " " + topic_name + " -1";
        i++;
        //std::getline(std::cin, line);

        zmq::message_t request(line.length());

        //Send
        memcpy(request.data(), line.c_str(), line.length());
        socket.send (request, zmq::send_flags::none);
        std::cout << "---Sent message: " << line.c_str() << std::endl;

        sleepForMs(500);

        //Get a reply
        zmq::message_t reply;
        std::cout << "...Waiting for reply" << std::endl;
        auto size = socket.recv (reply, zmq::recv_flags::none);
        char * reply_c_str = (char *) reply.data();
        reply_c_str[size.value()] = '\0';
        std::cout << "---Reply: " << reply_c_str << std::endl;

        
    }
    // END-Testing
}

void runClient(int clientID){

    zmq::context_t context(1);
    zmq::socket_t socket (context, zmq::socket_type::req);
    socket.connect("tcp://127.0.0.1:5555");

    std::map<std::string, int> subscribedTopics;

    while (true) {

        //Read from stdin
        std::string request;
        std::string instruction;
        std::cout << "Instruction Type?\n 1. SUB\n 2. UNSUB\n 3. GET\n 4. PUT\n"; 
        std::getline(std::cin, instruction);

        try {
            if (stoi(instruction) <= 4 || stoi(instruction) >= 1){
                request += instruction; //TODO PARSE TO STRING
            } else {
                std::cout << "Invalid instruction\n";
                continue;
            }
        }
        catch(const std::invalid_argument& e){
            std::cerr << "Invalid argument: " << e.what() << "\n";
            continue;
        }

        request += " " + std::to_string(clientID) + " ";

        std::string topic;
        std::cout << "Topic Type?\n"; 
        std::getline(std::cin, topic);

        request += topic + " ";

        switch (stoi(instruction)) {
            case 1:
                break;
            case 2:
                if(subscribedTopics.find(topic) == subscribedTopics.end()) {
                    std::cerr << "Topic not subscribed: \n";
                    continue;
                }
                break;
            case 3:
                request += std::to_string(getLastMessageIDFromTopic(subscribedTopics, topic));
                break;
            case 4:
                std::string content;
                std::cout << "Insert message:\n"; 
                std::getline(std::cin, content);
                request += content;
        }
        
        std::cout << "request: " << request << std::endl;

        zmq::message_t requestMsg(request.length());

        //Send
        memcpy(requestMsg.data(), request.c_str(), request.length());
        socket.send (requestMsg, zmq::send_flags::none);
        std::cout << "---Sent message: " << request.c_str() << std::endl;

        //Get a reply
        zmq::message_t reply;
        std::cout << "...Waiting for reply" << std::endl;
        auto size = socket.recv (reply, zmq::recv_flags::none);
        char * reply_c_str = (char *) reply.data();
        reply_c_str[size.value()] = '\0';
        std::cout << "---Reply: " << reply_c_str << std::endl;
    }
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
            //testClientCommunication(clientID);
            runClient(clientID);
        }
            return 0;
        default:
            printUsage();
            return 1;
    }

    return 0;
}
