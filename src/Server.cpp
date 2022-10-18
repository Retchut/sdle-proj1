#include <iostream>
#include <zmq.hpp>
#include <map>
#include <queue>

#include "Utils.h"
#include "Message.h"
#include "Topic.h"

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
std::map<int, std::string> messageMap;

void printUsage (){
    std::string usage = "Usage:\n\t./server";
    std::cout << usage << std::endl;
}

int run(std::map<std::string, Topic> * topics_map){

    zmq::context_t context (3);
    zmq::socket_t socket (context, zmq::socket_type::rep);
    socket.bind ("tcp://*:5555");
    while(true){
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

            //std::map<std::string, Topic>::iterator topic_pair = topics_map->find(topic_name);
            
            try {
                (*topics_map).at(topic_name);
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
                topic = &(*topics_map).at(topic_name);
            switch(instType){
                case SUB:
                    //SUB
                    if (topic->sub(client_id) == 0)
                        reply_msg = "SUB " + client_id + " " + topic_name;
                    else if (topic->sub(client_id) == 1)
                        reply_msg = "RESUB " + client_id + " " + topic_name;
                    else{
                        reply_msg = "PUB error";
                    }
                    break;
                case UNSUB:
                    //UNSUB
                    if (topic->unsub(client_id) == 0)
                        reply_msg = "UNSUB " + client_id + " " + topic_name;
                    else{
                        reply_msg = "UNSUB error";
                    }
                    break;
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
                        reply_msg = std::to_string(msg.get_id()) + " " + msg.get_content();
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
                    topics_map->insert(std::pair<std::string, Topic>(topic_name, new_topic));

                    reply_msg = "SUB " + client_id + " " + topic_name;
                    break;
            }
        }

        // Show topics_map
        for (auto it = (*topics_map).begin(); it!=(*topics_map).end(); ++it){
            std::cout << "->Topic " << it->first << ":\n";
            it->second.show();
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
