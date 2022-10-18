#include <iostream>
#include <stdexcept>
#include <string>
#include <map>
#include <zmq.hpp>
#include <thread>
#include <chrono>
#include <future>

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

void timeout(zmq::context_t & context, std::future<void> futureObj){
    int i = 0;
    while (i<10 && futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout)
    {
        sleepForMs(500);
        i++;
    }
    if (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
        try{
            context.shutdown();
        }catch (const std::exception & e){
            std::cout << "HERER" << std::endl;
        }
        std::cout << "Closed context" << std::endl;
    }
    std::cout << "Ending Thread" << std::endl;
}

void testClientCommunication(int clientID){
    // Testing
    zmq::context_t context(1);
    zmq::socket_t socket (context, zmq::socket_type::req);
    socket.connect("tcp://127.0.0.1:5555");

    std::thread * th;
    
    zmq::pollitem_t item;
    item.socket = socket;
    item.events = ZMQ_POLLIN;
    

    int i = -1;
    std::string topic_name;
    if (clientID == 1)
        topic_name = "2";
    else topic_name = "1";

    while(true){
        try{
            //Read from stdin
            std::string line;
            /*
            if (i > 2) i = 0;
            if (i == 0)
                line = "SUB " + std::to_string(clientID) + " " + topic_name;
            else if (i == 1)
                line = "PUT " + std::to_string(clientID) + " " + std::to_string(clientID) + " mensagem_catita" ;
            else if (i == 2)
                line = "GET " + std::to_string(clientID) + " " + topic_name + " -1";
            i++;
            */

            //std::cout << "Input message:" << std::endl;
            //std::getline(std::cin, line);

            
            if (i == -1)
                line = "SUB " + std::to_string(clientID) + " Topic1";
            else{
                line = "PUT  " + std::to_string(clientID) + " Topic1 Ola";
                for (int j = 0; j<10; ++j)
                    line += "Broo_";
            }
            std::cout << i << std::endl;
            i++;
            


            //Send
            zmq::message_t request(line.length());
            memcpy(request.data(), line.c_str(), line.length());
            socket.send (request, zmq::send_flags::none);
            std::cout << "---Sent message: " << line.c_str() << std::endl;

            //Get a reply
            //auto poll_res = zmq::poll(&item, 3);
            //std::cout << poll_res << std::endl;
            zmq::message_t reply;


            //std::thread timeoutThread(timeout);
            //timeoutThread.req

            std::promise<void> exitSignal; // Create a std::promise object
            std::future<void> futureObj = exitSignal.get_future();//Fetch std::future object associated with promise
            th = new std::thread(&timeout, std::ref(context), std::move(futureObj));// Starting Thread & move the future object in lambda function by reference

            std::cout << "...Waiting for reply" << std::endl;
            auto size = socket.recv (reply, zmq::recv_flags::none);
            char * reply_c_str = (char *) reply.data();
            reply_c_str[size.value()] = '\0';
            std::cout << "---Reply: " << reply_c_str << std::endl;
            std::cout << "Asking the thread to stop" << std::endl;

            exitSignal.set_value(); //Set the value 
            //(*th).join(); //Waiting for thread to be joined.
            //std::cout << "Thread joined" << std::endl;
            
            if (i == 200){
                std::cout << "Sleeping" << std::endl;
                sleepForMs(5000);
                i = 0;
            }
            
        }catch (const std::exception & e) {
            std::cout << "Catch: " << e.what() <<  std::endl;
            (*th).join();
            std::cout << "Thread joined" << std::endl;
            break;
        }
    }
    testClientCommunication(clientID);
    // END-Testing
}

void runClient(){

    zmq::context_t context(1);
    zmq::socket_t socket (context, zmq::socket_type::req);
    socket.connect("tcp://127.0.0.1:5555");

    std::map<std::string, int> subscribedTopics;

    while (true) {
        
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
            testClientCommunication(clientID);
            //runClient();
        }
            return 0;
        default:
            printUsage();
            return 1;
    }

    return 0;
}
