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

    void loadQueue(int clientID, std::string topicName);
    int sub(std::string client_id);
    int unsub(std::string client_id);

    int put(Message msg);
    Message get(std::string client_id, int last_msg_id);
    int rem(std::string client_id);

    std::string get_name();
    void show();

private:
    std::string name;
    std::map<std::string, std::queue <Message>> client_msg_queues;

};

#endif