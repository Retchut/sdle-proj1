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

    void loadQueue(int clientID, std::vector<int> messageIDs, std::vector<std::string> messageContents);
    int sub(int client_id);
    int unsub(int client_id);

    int put(Message msg);
    Message get(int client_id, int last_msg_id);
    int rem(int client_id);

    std::string get_name();
    void show();

private:
    std::string name;
    std::map<int, std::queue <Message>> client_msg_queues;

};

#endif