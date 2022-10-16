#ifndef _TOPIC_H_
#define _TOPIC_H_

#include <map>
#include <queue>
#include <string>

class Message;

class Topic{
public:
    Topic(std::string topic_name);
    ~Topic();

    int sub(std::string client_id);
    int unsub(std::string client_id);

    // puts a message in a topic (in every client's queue)
    int put(Message msg);

    int rem(std::string client_id);

    // 
    // last_msg_id: the ID of the last message the client has received
    //
    Message get(std::string client_id, int last_msg_id);
    void show();

private:
    std::string name;
    std::map<std::string, std::queue <Message>> client_msg_queues;

};

#endif