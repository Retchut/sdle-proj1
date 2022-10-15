#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>

void setupStorage(std::string entity);
int getNextPostID(std::string entityName, std::string topic);
int savePost(std::string entityName, std::string topic, std::string message, int postID);

#endif