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

std::condition_variable cv;
int flag;

void printUsage (){
    std::string usage = "Usage:\n\t./client <id>";
    std::cout << usage << std::endl;
}

void printSubscribedTopics (std::map<std::string, int> &topicIDs) {
    for (auto itr = topicIDs.begin(); itr != topicIDs.end(); ++itr) {
        std::cout << itr->first << '\t' << itr->second << '\n';
    }
}

void subscribeTopic (std::map<std::string, int> &topicIDs, std::string topic) {
    topicIDs.insert(std::pair<std::string, int>(topic, -1));
}

int unsubscribeTopic (std::map<std::string, int> &topicIDs, std::string topic) {
    return topicIDs.erase(topic);
}

int changeLastMessageID (std::map<std::string, int> &topicIDs, std::string topic, int messageID) {
    if(topicIDs.find(topic) != topicIDs.end()) {
        topicIDs.at(topic) = messageID;
        return 1;
    }
    return 0;
}

void timeout(zmq::context_t & context){
    std::mutex mtx;
    std::unique_lock<std::mutex> lck(mtx);
    while (cv.wait_for(lck,std::chrono::seconds(5))==std::cv_status::timeout) {
        std::cout << "Timeout" << std::endl;
        context.shutdown(); 
        std::cout << "Closed context" << std::endl;
        std::cout << "Ending Thread" << std::endl;
        return;
    }
    if (flag == 0){
        context.shutdown(); 
        std::cout << "Closed context" << std::endl;
    }
    std::cout << "Ending Thread" << std::endl;
}

int testClientCommunication(int clientID){
    // Testing
    zmq::context_t context(1);
    zmq::socket_t socket (context, zmq::socket_type::req);
    socket.connect("tcp://127.0.0.1:5555");

    std::thread * th;

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
                for (int j = 0; j<100; ++j)
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
            flag = 0;
            th = new std::thread(timeout, std::ref(context));

            zmq::message_t reply;
            std::cout << "...Waiting for reply" << std::endl;
            auto size = socket.recv (reply, zmq::recv_flags::none);
            flag = 1;
            cv.notify_one(); 

            char * reply_c_str = (char *) reply.data();
            reply_c_str[size.value()] = '\0';
            std::cout << "---Reply: " << reply_c_str << std::endl;
            std::cout << "Asking the thread to stop" << std::endl;

            (*th).join(); //Waiting for thread to be joined.
            std::cout << "Thread joined" << std::endl;
            
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
    return 0;
}

int runClient(std::map<std::string, int> &nextTopicIDs){

    zmq::context_t context(1);
    zmq::socket_t socket (context, zmq::socket_type::req);
    socket.connect("tcp://127.0.0.1:5555");


    // while (true) {
        
    // }
    return 0;
}

int main (int argc, char *argv[]) {
    if(argc == 2){
        try {
                clientID = std::stoi(std::string(argv[1]));
            }
            catch(const std::invalid_argument& e){
                std::cerr << "Invalid argument: " << e.what() << "\n";
                printUsage();
                return 1;
            }
            entityName = "Client" + std::to_string(clientID);
            std::map<std::string, int> topicIDs;
            // checks if the storage location already exists
            if(setupStorage(entityName)){
                // if the storage location exists, resumes operation from that data (subscribing all topics)
                if(loadClient(entityName, topicIDs)){
                    std::cout << "An error occured while loading the client's data after crashing" << std::endl;
                    return 1;
                }
            }

            std::cout << "Running client " << clientID << std::endl;
            // return testClientCommunication(clientID);
            // return runClient(topicIDs);
    }
    else{
        printUsage();
        return 1;
    }
}
