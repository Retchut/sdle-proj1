#include <iostream>
#include <map>
#include "Message.h"

Message::Message(int id, std::string content)
    :id(id), content(content)
{}
Message::Message(std::string content)
    :content(content)
{
    id = Message::get_next_message_id();
}
Message::~Message(){}

int Message::get_id(){
    return id;
}

std::string Message::get_content(){
    return content;
}

int Message::get_next_message_id(){
    if (next_message_id <0)
        next_message_id = 0;
    return next_message_id++;
}

void Message::show(){
    std::cout << "Message " << id << ": " << content << std::endl;
}

int Message::next_message_id = 0;