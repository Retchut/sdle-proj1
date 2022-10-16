#include <iostream>
#include <map>
#include "Message.h"

Message::Message(std::string id, std::string content)
    :id(id), content(content)
{}
Message::~Message(){}

std::string Message::get_id(){
    return id;
}

std::string Message::get_content(){
    return content;
}