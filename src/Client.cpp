#include <iostream>
#include <stdexcept>
#include <string>
#include <map>
#include <zmq.hpp>
#include <thread>
#include <chrono>
#include <future>
#include <filesystem>
#include <sstream>
#include <fstream>

#include "Utils.h"

namespace fs = std::filesystem;

extern std::string STORAGE_DIR;

enum InstructionType {
    INVALID_INSTRUCTION = -1,
    SUB = 1,
    UNSUB = 2,
    GET = 3,
    PUT = 4,
    SUB_NEW_TOPIC = 5,
    RESUB = 6
};

InstructionType getInstructionType(const char* instruction) {

    InstructionType instType;

    if (strcmp(instruction, "SUB") == 0) instType = SUB;
    else if (strcmp(instruction, "UNSUB")  == 0) instType = UNSUB;
    else if (strcmp(instruction, "GET")  == 0) instType = GET;
    else if (strcmp(instruction, "PUT")  == 0) instType = PUT;
    else if (strcmp(instruction, "RESUB")  == 0) instType = RESUB;
    else instType = INVALID_INSTRUCTION;

    return instType;
}

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
    auto it = topicIDs.find(topic);
    topicIDs.erase(it);
    return 0;
}

int changeLastMessageID (std::map<std::string, int> &topicIDs, std::string topic, int messageID) {
    if(topicIDs.find(topic) != topicIDs.end()) {
        topicIDs.at(topic) = messageID;
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

void timeout(zmq::context_t & context){
    std::mutex mtx;
    std::unique_lock<std::mutex> lck(mtx);
    while (cv.wait_for(lck,std::chrono::seconds(5))==std::cv_status::timeout) {
        std::cout << "Timeout" << std::endl;
        context.shutdown(); 
        //std::cout << "Closed context" << std::endl;
        //std::cout << "Ending Thread" << std::endl;
        return;
    }
    if (flag == 0){
        context.shutdown(); 
        std::cout << "Closed context" << std::endl;
    }
    //std::cout << "Ending Thread" << std::endl;
}

int loadClient(std::string entity, std::map<std::string, int> &subscribedTopics){
    std::string storageDirectory = STORAGE_DIR + "/" + entity + "/";

    try{
        fs::directory_iterator it = fs::directory_iterator(storageDirectory);

        // iterate through entries in the client directory
        for(const auto &entry : it){
            if(!entry.is_directory()){
                std::string topicName = fs::path(entry).filename();
                //read last ID post from topic file
                std::ifstream iss(storageDirectory + topicName);
                std::string lastIDPost;
                iss >> lastIDPost;

                subscribedTopics.insert({topicName, stoi(lastIDPost)});
            }
        }
        return 0;
    }
    catch(const std::exception & e){
        std::cout << "Caught exception: " << e.what() << "\n";
    }
    return 1;
}

int testClientCommunication(int clientID){
    // Testing
    zmq::context_t context(1);
    zmq::socket_t socket (context, zmq::socket_type::req);
    socket.connect("tcp://127.0.0.1:5555");

    std::thread * th;
    std::map<std::string, int> subscribedTopics;

    //int i = 0;
    while(true){
        try{
            //Read from stdin
            std::string line;

            std::cout << "Input message:" << std::endl;
            std::getline(std::cin, line);

            if (line.length() == 0)
            {
                std::cout << "Invalid command" << std::endl;
                continue;
            }

            // parse input
            char *cstr = new char[line.length() + 1];
            strcpy(cstr, line.c_str());
            auto tokens = tokenize(cstr);

            InstructionType instType = INVALID_INSTRUCTION;


            if (tokens[0] == "SUB"){
            if (tokens.size() < 3){
                std::cout << "Invalid number of arguments for SUB" << std::endl;
                continue;
            }
            else instType = SUB;
            }
            else if (tokens[0] == "UNSUB"){
                if (tokens.size() < 3){
                    std::cout << "Invalid number of arguments for UNSUB" << std::endl;
                    continue;
                }
                else instType = UNSUB;
            }
            else if (tokens[0] == "GET"){
                if (tokens.size() < 4){
                    std::cout << "Invalid number of arguments for GET" << std::endl;
                    continue;
                }
                else instType = GET;
            }
            else if (tokens[0] == "PUT"){
                if (tokens.size() < 4){
                    std::cout << "Invalid number of arguments for PUT" << std::endl;
                    continue;
                }
                else instType = PUT;
            }
            else {
                std::cout << "Invalid instruction" << std::endl;
                continue;
            }

            /*
            if (i == -1)
                line = "SUB " + std::to_string(clientID) + " Topic1";
            else{
                line = "PUT  " + std::to_string(clientID) + " Topic1 Ola";
                for (int j = 0; j<100; ++j)
                    line += "Broo_";
            }
            std::cout << i << std::endl;
            i++;
            */

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
            //std::cout << "Asking the thread to stop" << std::endl;

            // Parse reply

            (*th).join(); //Waiting for thread to be joined.
            //std::cout << "Thread joined" << std::endl;
            
            /*
            if (i == 200){
                std::cout << "Sleeping" << std::endl;
                sleepForMs(5000);
                i = 0;
            } */
            
        }catch (const std::exception & e) {
            //std::cout << "Catch: " << e.what() <<  std::endl;
            (*th).join();
            //std::cout << "Thread joined" << std::endl;
            break;
        }
    }
    testClientCommunication(clientID);
    return 0;
}

int checkInstruction(int argc, char *argv[]){

    if(argc == 1) {
        std::cout << "Invalid instruction." << std::endl;
        return 1;
    }
    InstructionType instType = getInstructionType(argv[1]);

    switch (instType) {
        case SUB:
        case UNSUB:
            if(argc != 4){
                std::cout << "Invalid number of arguments for " << argv[1] << std::endl;
                return 1;
            }
        break;
        case GET:
            if(argc != 4){
                std::cout << "Invalid number of arguments for " << argv[1] << std::endl;
                return 1;
            }
        break;
        case PUT:
            if(argc < 5){
                std::cout << "Invalid number of arguments for " << argv[1] << std::endl;
                return 1;
            }
        break;
        case INVALID_INSTRUCTION:
        default:
            std::cout << "Invalid instruction." << std::endl;
            return 1;
        break;
    }

    return 0;
}

void parseReply(char* reply, int replySize, std::map<std::string, int> &subscribedTopics) {
    // get request string

    if(strcmp(reply, "error_2") == 0) {
        std::cout << "No messages to show yet\n";
    } 

    std::stringstream ss(reply);

    std::string instruction;
    std::string clientID;
    std::string topic;
    std::string postid;
    std::string content;
    ss >> instruction >> clientID >> topic;

    switch (getInstructionType(instruction.c_str())){
        case SUB:
            subscribeTopic(subscribedTopics, topic);
            std::cout << topic << " subscribed with success\n";
            break;
        case UNSUB:
            unsubscribeTopic(subscribedTopics, topic);
            std::cout << topic << " unsubscribed with success\n";
        case RESUB:
            if(subscribedTopics.find(topic) == subscribedTopics.end()){
                subscribeTopic(subscribedTopics, topic);
            } else {
                std::cout << "Already subscribed to topic:\n" << topic << std::endl;
            }
        case GET:
            ss >> postid >> content;
            std::cout << "Message received from topic " << topic << ": " << content << std::endl;
            changeLastMessageID(subscribedTopics, topic, stoi(postid));
            break;
        case PUT:
            std::cout << "Posted message with success\n";
            break;
        default:
            break;
    }
}

void updateFiles(std::map<std::string, int> &subscribedTopics, std::string entityName) {
    std::string path = STORAGE_DIR + "/" + entityName + "/";

    for(const auto &topic: subscribedTopics) {
        
        std::ofstream fileStream;
        fileStream.open(path + topic.first); 

        if(!fileStream) {
            std::cout << "File " << path + topic.first << " couldn't be opened" << std::endl;
            return;
        }

        fileStream << topic.second;
        fileStream.close();
    }
}

void runClient(char* argv[], int argc){

    std::string clientID = argv[2];
    entityName = "Client" + clientID;
    std::map<std::string, int> subscribedTopics;

    // checks if the storage location already exists
    if(setupStorage(entityName)){
        // if the storage location exists, resumes operation from that data (subscribing all topics)
        if(loadClient(entityName, subscribedTopics)){
            std::cout << "An error occured while loading the client's data after crashing" << std::endl;
            return;
        }

    }

    char *instruction = argv[1];
    char *topic = argv[3];

    //Check if instruction is valid for a certain client and its subscribed topics
    switch (getInstructionType(instruction)) {
        case SUB:
        case PUT:
            break;
        case UNSUB:
        case GET:
            if(subscribedTopics.find(topic) == subscribedTopics.end()) {
                std::cout << "Topic not subscribed: " << topic << std::endl;
                return;
            }
            break;
        default:
            break;
    }
    std::thread * th;
    try {
        zmq::context_t context(1);
        zmq::socket_t socket (context, zmq::socket_type::req);
        socket.connect("tcp://127.0.0.1:5555");
        
        // get request string
        std::stringstream ss;
        std::string sep = " ";
    
        for(int i=1; i<argc; ++i) {
            ss << argv[i] << sep;
        }
        std::string request = ss.str();

        //Add last post ID in GET request
        if(getInstructionType(instruction) == GET) {
            request += std::to_string(getLastMessageIDFromTopic(subscribedTopics, topic));
            std::cout << request;
        }

        zmq::message_t requestMsg(request.length());

        //Send
        std::cout << request << std::endl;
        memcpy(requestMsg.data(), request.c_str(), request.length());
        socket.send (requestMsg, zmq::send_flags::none);
        std::cout << "---Sent message: " << request.c_str() << std::endl;

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
        //std::cout << "---Reply: " << reply_c_str << std::endl;
        //std::cout << "Asking the thread to stop" << std::endl;
        
        (*th).join(); //Waiting for thread to be joined.
        //std::cout << "Thread joined" << std::endl;

        parseReply(reply_c_str, size.value(), subscribedTopics);
        updateFiles(subscribedTopics, entityName);
    }
    catch (const std::exception & e) {
        std::cout << "Catch: " << e.what() <<  std::endl;
        (*th).join();
        std::cout << "Thread joined" << std::endl;
    }

}


int main (int argc, char *argv[]) {
    
    if (checkInstruction(argc, argv)) {
        return 1;
    }

    runClient(argv, argc);

    return 0;
}
