#include <iostream>
#include <zmq.hpp>
#include <map>
#include <queue>
#include "Utils.h"

#include "Message.h"
#include "Topic.h"


Topic::Topic(std::string topic_name): name(topic_name){};
Topic::~Topic(){
    //std::cout << "Deconstructing " + this->name << std::endl;
};


void Topic::loadQueue(int clientID, std::vector<int> messageIDs, std::vector<std::string> messageContents){

    auto client_queue = &client_msg_queues.at(clientID);

    for (int i = 0; i < messageIDs.size(); ++i){
        auto new_message = Message(messageIDs[i], messageContents[i]);
        (*client_queue).push(new_message);
    }
}

int Topic::sub(int client_id){
    try {
        client_msg_queues.at(client_id);
        // client is already subscribed (i.e. has a queue)
        return 1;
    }catch(const std::exception & e){
        //client_msg_queues[client_id] = std::queue<Message>();
        client_msg_queues.insert(std::pair<int, std::queue<Message>>(client_id, std::queue<Message>()));
        client_msg_queues.at(client_id);
    }
    return 0;
}
int Topic::unsub(int client_id){
    if (client_msg_queues.erase(client_id) == 0){
        // client not subscribed
        return 1;
    }
    else{
        if (client_msg_queues.size() == 0)
            return 2;
        return 0;
    }
}

// unnecessary?
int Topic::rem(int client_id){
    client_msg_queues.erase(client_id);
    return 0;
}

// puts a message in a topic (in every client's queue)
int Topic::put(Message msg){
    for (std::map<int, std::queue <Message>>::iterator it = client_msg_queues.begin(); it!=client_msg_queues.end(); ++it){
        it->second.push(msg);
        subscriberFilePush(name, std::to_string(it->first), std::to_string(msg.get_id()));
        //std::cout << "Pushed " << it->second.back().get_content() << std::endl;
    }
    return 0;
}

// 
// last_msg_id: the ID of the last message the client has received
//
Message Topic::get(int client_id, int last_msg_id){
    std::queue<Message> * queue;
    try {
        queue = &client_msg_queues.at(client_id);
    }catch(const std::exception & e){
        return Message(-1, "");
    }

    if (queue->size() > 0 && queue->front().get_id() == last_msg_id){
        queue->pop();
        std::cout << "2" << std::endl;
        
        std::cout << name << " " << client_id << std::endl;
        subscriberFilePop(name, std::to_string(client_id));
    }

    if (queue->size() < 1)
        return Message(-2, "");
    return queue->front();

}

std::string Topic::get_name(){
    return name;
}

void Topic::show(){
    // show client ids and their queue size
    for (std::map<int, std::queue <Message>>::iterator it = client_msg_queues.begin(); it!=client_msg_queues.end(); ++it){
        std::cout << "client " << it->first << ": " << it->second.size() << std::endl;
        if (it->second.size() > 0){
            std::cout << "_front: ";
            it->second.front().show();
            std::cout << "_back: ";
            it->second.back().show();
        }
    }
}