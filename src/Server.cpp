#include <iostream>
#include <zmq.hpp>
#include <map>
#include <queue>
#include "Utils.h"

#include "Message.h"
#include "Topic.h"

int THREAD_NUM = 2;
std::string entityName;
std::map<int, std::string> messageMap;

void printUsage (){
    std::string usage = "Usage:\n\t./server";
    std::cout << usage << std::endl;
}

int savePost(std::string folderName, std::string topic, std::string message){

    return 0;
}

std::vector <std::string> tokenize(char * input){
    std::vector<std::string> res;
    char *token = std::strtok(input, " ");

    while (token != NULL){
        res.push_back(std::string(token));
        //std::cout << token << std::endl;
        token = strtok(NULL, " ");
    }
    return res;
}

int run(std::map<std::string, Topic> * topics_map){
    

    zmq::context_t context (2);
    zmq::socket_t socket (context, zmq::socket_type::rep);
    socket.bind ("tcp://*:5555");
    while(true){
        zmq::message_t request;

        std::cout << "waiting for message" << std::endl;
        auto size = socket.recv (request, zmq::recv_flags::none);
        std::cout << "Received msg size: " << size.value() << std::endl;

        // add '\0'
        char * request_data_c_str = (char *) request.data();
        request_data_c_str[size.value()] = '\0';

        std::string request_data(request_data_c_str);        
        std::string reply_msg;
        int inst_type = -1; // instruction type
        std::vector <std::string> tokens = tokenize((char *) request.data());

        for (int i = 0; i<tokens.size(); ++i)
            std::cout << "token " << i << ": " << tokens[i] << std::endl;



        if (tokens[0] == "SUB"){
            if (tokens.size() < 3){
                reply_msg = "Invalid number of arguments for SUB";
                inst_type = -1;
            }
            else inst_type = 1;
        }
        else if (tokens[0] == "UNSUB"){
            if (tokens.size() < 3){
                reply_msg = "Invalid number of arguments for UNSUB";
                inst_type = -1;
            }
            else inst_type = 2;
        }
        else if (tokens[0] == "GET"){
            if (tokens.size() < 4){
                reply_msg = "Invalid number of arguments for GET";
                inst_type = -1;
            }
            else inst_type = 3;
        }
        else if (tokens[0] == "PUT"){
            if (tokens.size() < 4){
                reply_msg = "Invalid number of arguments for PUT";
                inst_type = -1;
            }
            else inst_type = 4;
        }
        else {
            reply_msg = "error";
        }

        if (inst_type != -1 && tokens.size() > 2){
            std::string client_id;
            std::string topic_name;
            std::string last_msg_id;
            std::string content;

            // read first two arguments (which are common for all instruction types)
            for (int i=0; i<2; ++i){    
                if (i == 0)
                    client_id = tokens[i];
                else // i == 1
                    topic_name = tokens[i];
            }

            Topic *topic = &topics_map->find(topic_name)->second;
            switch(inst_type){
                case 1:
                    break;
                case 2:
                    break;
                case 3:
                {
                    last_msg_id = tokens[3];
                    Message msg = topic->get(client_id, last_msg_id);
                    break;
                }
                case 4:
                    break;
            }
        }
        std::cout << "reply: " << reply_msg.c_str() << std::endl;
        zmq::message_t reply ( reply_msg.length());
        memcpy (reply.data (), reply_msg.c_str(), reply_msg.length());
        socket.send (reply, zmq::send_flags::none);
        //std::cout << "Sent: " << reply_msg.c_str() << std::endl;

    }
    return 0;
}

int main (int argc, char *argv[]) {
    entityName = "server";
    setupStorage(entityName);
    std::map<std::string, Topic> topics_map;

    switch(argc){
        case 1:
            // run server, receiving connections
            std::cout << "Running server" << std::endl;
            return run(&topics_map);
        default:
            printUsage();
            return 1;
    }

    return 0;
}
