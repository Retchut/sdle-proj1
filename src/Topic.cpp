#include <iostream>
#include <zmq.hpp>
#include <map>
#include <queue>
#include "Utils.h"

#include "Message.h"
#include "Topic.h"


Topic::Topic(std::string topic_name): name(topic_name){};
Topic::~Topic(){};

int Topic::sub(std::string client_id){
    return 0;
}
int Topic::unsub(std::string client_id){
    return 0;
}

    // puts a message in a topic (in every client's queue)
int Topic::put(Message msg){
    for (std::map<std::string, std::queue <Message>>::iterator it = client_msg_queues.begin(); it!=client_msg_queues.end(); ++it){
        it->second.push(msg);
    }
    return 0;
}

int Topic::rem(std::string client_id){
    client_msg_queues.erase(client_id);
    return 0;
}

// 
// last_msg_id: the ID of the last message the client has received
//
Message Topic::get(std::string client_id, std::string last_msg_id){
    auto queue = &client_msg_queues.find(client_id)->second;
    Message front_message = queue->front();
    if (front_message.get_id() == last_msg_id){
        queue->pop();
        front_message = queue->front();
    }
    return front_message;
}
