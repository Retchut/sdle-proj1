#include <iostream>
#include <zmq.hpp>
#include <vector>
#include <map>
#include <queue>
#include <filesystem>
#include <fstream>

#include "Utils.h"
#include "Topic.h"
#include "Message.h"

namespace fs = std::filesystem;

extern std::string STORAGE_DIR;

enum InstructionType {
    INVALID_INSTRUCTION = -1,
    SUB = 1,
    UNSUB = 2,
    GET = 3,
    PUT = 4,
    SUB_NEW_TOPIC = 5
};

int THREAD_NUM = 2;
std::string entityName;

void printUsage (){
    std::string usage = "Usage:\n\t./Server";
    std::cout << usage << std::endl;
}

void deleteSubscriberFile(std::string topicName, int clientID){
    std::string topicStorageDir = STORAGE_DIR + "/" + entityName + "/" + topicName + "/";
    std::string topicSubDir = STORAGE_DIR + "/Subscribers/" + topicName + "/";
    std::string subscriberFile = topicSubDir + std::to_string(clientID);

    try{
        if(!fs::exists(subscriberFile)){
            std::cout << "Subscriber file for client with ID number " << clientID << " for topic " << topicName << " does not exist." << std::endl;
            return;
        }
        fs::remove(subscriberFile);

        // if there are no subscribers, we clean up the storage directory
        if(fs::is_empty(topicSubDir)){
            if(!fs::exists(topicSubDir)){
                std::cout << "Subscriber directory for topic " << topicName << " does not exist." << std::endl;
                return;
            }
            
            fs::remove(topicSubDir); // removes empty directory

            if(fs::exists(topicStorageDir)){
                fs::remove_all(topicStorageDir); // removes storage directory of this topic, recursively
            }
        }
    }
    catch(const std::exception & e){
        std::cout << "Caught exception: " << e.what() << "\n";
    }
}

void addSubscriber(std::string topicName, int clientID){
    std::string subscribersDirectory = STORAGE_DIR + "/Subscribers/" + topicName + "/";

    try{
        std::cout << "Adding subscriber" << std::endl;
        std::string path = subscribersDirectory + std::to_string(clientID);
        std::ofstream ofs(subscribersDirectory);
        ofs.close();
    }
    catch(const std::exception & e){
        std::cout << "Caught exception: " << e.what() << "\n";
    }
}

void createSubscribersFile(std::string topic, int clientID){
    std::string subscribersDir = STORAGE_DIR + "/Subscribers/" + topic + "/";
    try{
        if(!fs::exists(subscribersDir))
            fs::create_directories(subscribersDir);
        std::string path = subscribersDir + std::to_string(clientID);
        std::ofstream ofs(path);
        ofs.close();
    }
    catch(const std::exception & e){
        std::cout << "Caught exception: " << e.what() << "\n";
    }
}

std::map<std::string, std::vector<int>> recoverSubscribers(std::string subscribersDirectory){
        // iterate through entries in the subscribers directory
        // and build map mapping the topic strings to the topic subscribers
    std::map<std::string, std::vector<int>> subscriberMap;
    
    try {
        fs::directory_iterator subIt = fs::directory_iterator(subscribersDirectory);
        for(const auto &entry : subIt){
            std::cout << "here rec subs" << std::endl;
            if(entry.is_directory()){
                fs::directory_iterator topicIt = fs::directory_iterator(entry.path());
                std::string topicName = entry.path().filename();

                std::cout << topicName << std::endl;

                //subscriberMap.insert({ topicName, std::vector<int>() });
                subscriberMap.insert(std::pair<std::string, std::vector<int>>(topicName, std::vector<int>()));
                
                auto subMapEntry = subscriberMap.find(topicName);
                for(const auto &client : topicIt){
                    int clientID = std::stoi(client.path().filename());
                    subMapEntry->second.push_back(clientID);
                    
                    std::cout << subMapEntry->second.size() << std::endl;
                }
            }
        }
    }
    catch(const std::exception & e){
        std::cout << "Caught exception: " << e.what() << "\n";
    }

    return subscriberMap;
}

int loadServer(std::string entity, std::map<std::string, Topic> &topicMap, std::map<std::string, int> &pubInts){
    std::string storageDirectory = STORAGE_DIR + "/" + entity + "/";

    try{
        std::string subscribersDirectory = STORAGE_DIR + "/Subscribers/";
        std::map<std::string, std::vector<int>> subMap = recoverSubscribers(subscribersDirectory);

        // iterate through entries in the server directory
        if (!subMap.empty()){
            fs::directory_iterator it = fs::directory_iterator(storageDirectory);
            for(const auto &entry : it){
                if(entry.is_directory()){
                    std::string topicName = fs::path(entry).filename();
                    auto topicSubIt = subMap.find(topicName);


                    Topic topicObj = Topic(topicName);

                    for(int i = 0; i < topicSubIt->second.size(); i++){
                        // subscribe
                        int subbedID = topicSubIt->second.at(i);
                        topicObj.sub(std::to_string(subbedID));

                        std::string subFileContents = subscriberFileRead(topicName, std::to_string(subbedID));
                        topicObj.loadQueue(subbedID, subFileContents);
                        std::cout << "loaded queues for topic " << topicName << " for subscriber " <<  i << std::endl;
                    }

                    int nextPubID = getNextPostID(entity, topicName);
                    topicMap.insert({ topicName, topicObj });
                    pubInts.insert({ topicName, nextPubID });
                    
                }
            }
        }
        std::cout << "Successfully loaded server" << std::endl;
        return 0;
    }
    catch(const std::exception & e){
        std::cout << "Caught exception: " << e.what() << "\n";
    }
    return 1;
}

int run(std::map<std::string, Topic> * topicsMap, std::map<std::string, int> * pubInts){

    zmq::context_t context (THREAD_NUM);
    zmq::socket_t socket (context, zmq::socket_type::rep);
    socket.bind ("tcp://*:5555");
    while(true){
        // Show topicsMap
        for (auto it = (*topicsMap).begin(); it!=(*topicsMap).end(); ++it){
            std::cout << "->Topic " << it->first << ":\n";
            it->second.show();
        }

        zmq::message_t request;

        std::cout << "...Waiting for message" << std::endl;
        auto size = socket.recv (request, zmq::recv_flags::none);
        std::cout << "---Received msg size: " << size.value() << std::endl;

        // add '\0'
        char * request_data_c_str = (char *) request.data();
        request_data_c_str[size.value()] = '\0';

        std::string request_data(request_data_c_str);        
        std::string reply_msg = "empty";
        InstructionType instType = INVALID_INSTRUCTION; // instruction type
        std::vector <std::string> tokens = tokenize((char *) request.data());

        //printTokens(tokens);

        if (tokens[0] == "SUB"){
            if (tokens.size() < 3){
                reply_msg = "Invalid number of arguments for SUB";
                instType = INVALID_INSTRUCTION;
            }
            else instType = SUB;
        }
        else if (tokens[0] == "UNSUB"){
            if (tokens.size() < 3){
                reply_msg = "Invalid number of arguments for UNSUB";
                instType = INVALID_INSTRUCTION;
            }
            else instType = UNSUB;
        }
        else if (tokens[0] == "GET"){
            if (tokens.size() < 4){
                reply_msg = "Invalid number of arguments for GET";
                instType = INVALID_INSTRUCTION;
            }
            else instType = GET;
        }
        else if (tokens[0] == "PUT"){
            if (tokens.size() < 4){
                reply_msg = "Invalid number of arguments for PUT";
                instType = INVALID_INSTRUCTION;
            }
            else instType = PUT;
        }
        else {
            reply_msg = "error";
        }

        if (instType != INVALID_INSTRUCTION && tokens.size() > 2){
            std::string client_id;
            std::string topic_name;
            int last_msg_id;
            std::string content;

            // read first two arguments (which are common for all instruction types)
            client_id = tokens[1];
            topic_name = tokens[2];

            //std::map<std::string, Topic>::iterator topic_pair = topicsMap->find(topic_name);
            
            try {
                (*topicsMap).at(topic_name);
            }catch(const std::exception & e){
                // topic doesn't exist
                if (tokens[0] == "SUB")
                    instType = SUB_NEW_TOPIC; // to create a new topic
                else{
                    std::cout << "Topic does not exist" << std::endl;
                    reply_msg = "Invalid topic name";
                    instType = INVALID_INSTRUCTION;
                }
            }
            Topic * topic;
            if (instType != INVALID_INSTRUCTION && instType != SUB_NEW_TOPIC)
                topic = &(*topicsMap).at(topic_name);
            switch(instType){
                case SUB:
                    //SUB
                    if (topic->sub(client_id) == 0){
                        reply_msg = "SUB " + client_id + " " + topic_name;
                        int client_idInt;
                        try{
                            client_idInt = stoi(client_id);
                            addSubscriber(topic_name, stoi(client_id));
                        }
                        catch(std::exception e){
                            std::cout << "Caught exception while attempting to add a subscriber: " << e.what() << std::endl;
                        }
                    }
                    else if (topic->sub(client_id) == 1)
                        reply_msg = "RESUB " + client_id + " " + topic_name;
                    else{
                        reply_msg = "SUB error";
                    }
                    break;
                case UNSUB:
                    //UNSUB
                {
                    int unsub_res = topic->unsub(client_id);
                    if (unsub_res == 0){
                        deleteSubscriberFile(topic_name, std::stoi(client_id));
                        reply_msg = "UNSUB " + client_id + " " + topic_name;
                    }else if (unsub_res == 2){
                        (*topicsMap).erase(topic_name); // Delete topic from topicsMap (this client was the last subscriber)
                        deleteSubscriberFile(topic_name, std::stoi(client_id));
                        reply_msg = "UNSUB " + client_id + " " + topic_name;
                    }
                    else{
                        reply_msg = "UNSUB error";
                    }
                    break;
                }
                case GET:
                    //GET
                {
                    if (tokens[3] == "null")
                        last_msg_id = -1;
                    else {
                        try {
                            last_msg_id = std::stoi(tokens[3]);
                        }
                        catch(const std::invalid_argument& e){
                            std::cout << "Invalid argument: " << e.what() << std::endl;
                            reply_msg = "Invalid last message id";
                            break;
                        }
                    }
                    Message msg = topic->get(client_id, last_msg_id);
                    if (msg.get_id() != -1 && msg.get_id() != -2)
                        reply_msg = "GET " + client_id + " " + topic_name + " " + std::to_string(msg.get_id()) + " " + msg.get_content();
                    else if(msg.get_id() == -1)
                        reply_msg = "error_1";
                    else if(msg.get_id() == -2)
                        reply_msg = "error_2";
                    else 
                        reply_msg == "error"; //this shouldn't happen
                    break;
                }
                case PUT:
                    //PUT
                {
                    content = tokens[3];

                    int next_post_id = getNextPostID(entityName, topic_name);
                    Message new_message = Message(next_post_id, content);
                    topic->put(new_message);

                    savePost(entityName, topic->get_name(), content, new_message.get_id());
                    reply_msg = "PUT " + client_id + " " + topic_name + " " + new_message.get_content();
                    break;
                }
                case SUB_NEW_TOPIC:
                    //SUB to nonexistent topic
                    Topic new_topic = Topic(topic_name);
                    new_topic.sub(client_id);
                    topicsMap->insert(std::pair<std::string, Topic>(topic_name, new_topic));
                    createSubscribersFile(topic_name, stoi(client_id));

                    reply_msg = "SUB " + client_id + " " + topic_name;
                    break;
            }
        }

        //std::cout << "___sleeping___" << std::endl;
        //sleep(3);
        //std::cout << "woke_up" << std::endl;
        
        std::cout << "---Reply: " << reply_msg.c_str() << std::endl;
        zmq::message_t reply ( reply_msg.length());
        memcpy (reply.data (), reply_msg.c_str(), reply_msg.length());
        socket.send (reply, zmq::send_flags::none);
        //std::cout << "Sent: " << reply_msg.c_str() << std::endl;

    }
    return 0;
}

int main (int argc, char *argv[]) {
    if(argc == 1){
        entityName = "Server";
        std::map<std::string, Topic> topicsMap;
        std::map<std::string, int> pubInts;
        // checks if the storage location already exists
        if(setupStorage(entityName)){
            // if the storage location exists, resumes operation from that data
            if(loadServer(entityName, topicsMap, pubInts)){
                std::cout << "An error occured while loading the server's data after crashing" << std::endl;
                return 1;
            }
            // missing resubscribing client for every topic?
            //      requires saving when a client subscribes/unsubscribes in a directory then reading?
        }
        
        std::cout << "Running server" << std::endl;
        return run(&topicsMap, &pubInts);
    }
    else{
        printUsage();
        return 1;
    }
}
