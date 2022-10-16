#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <string>

class Message{
public:
    Message(std::string id, std::string content);
    ~Message();
    std::string get_id();
    std::string get_content();
private:
    std::string id;
    std::string content;
};

#endif