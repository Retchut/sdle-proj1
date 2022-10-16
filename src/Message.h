#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <string>

class Message{
public:
    Message(int id, std::string content);
    Message(std::string content);
    ~Message();

    int get_id();
    std::string get_content();
    static int get_next_message_id();
    void show();

private:
    int id;
    std::string content;
    static int next_message_id;
};

#endif