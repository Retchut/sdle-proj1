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

    auto clientID_str = std::to_string(clientID);
    auto client_queue = client_msg_queues.at(clientID_str);

    for (int i = 0; i < messageIDs.size(); ++i){
        auto new_message = Message(messageIDs[i], messageContents[i]);
        client_queue.push(new_message);
        new_message.show();
    }


}

int Topic::sub(std::string client_id){
    try {
        client_msg_queues.at(client_id);
        // client is already subscribed (i.e. has a queue)
        return 1;
    }catch(const std::exception & e){
        //client_msg_queues[client_id] = std::queue<Message>();
        client_msg_queues.insert(std::pair<std::string, std::queue<Message>>(client_id, std::queue<Message>()));
        client_msg_queues.at(client_id);
    }
    return 0;
}
int Topic::unsub(std::string client_id){
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
int Topic::rem(std::string client_id){
    client_msg_queues.erase(client_id);
    return 0;
}

// puts a message in a topic (in every client's queue)
int Topic::put(Message msg){
    for (std::map<std::string, std::queue <Message>>::iterator it = client_msg_queues.begin(); it!=client_msg_queues.end(); ++it){
        it->second.push(msg);
        //std::cout << "Pushed " << it->second.back().get_content() << std::endl;
    }
    return 0;
}

// 
// last_msg_id: the ID of the last message the client has received
//
Message Topic::get(std::string client_id, int last_msg_id){
    std::queue<Message> * queue;
    try {
        queue = &client_msg_queues.at(client_id);
    }catch(const std::exception & e){
        return Message(-1, "");
    }

    Message front_message = queue->front();
    if (queue->size() > 0 && front_message.get_id() == last_msg_id){
        queue->pop();
        front_message = queue->front();
    }

    if (queue->size() < 1)
        return Message(-2, "");

    return front_message;
}

std::string Topic::get_name(){
    return name;
}

void Topic::show(){
    // show client ids and their queue size
    for (std::map<std::string, std::queue <Message>>::iterator it = client_msg_queues.begin(); it!=client_msg_queues.end(); ++it){
        std::cout << "client " << it->first << ": " << it->second.size() << std::endl;
        if (it->second.size() > 0){
            std::cout << "_front: ";
            it->second.front().show();
            std::cout << "_back: ";
            it->second.back().show();
        }
    }
}