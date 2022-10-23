#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>

class Topic;

void sleepForMs(float miliseconds);
int setupStorage(std::string entity);
std::string subscriberFileRead(std::string topicName, std::string clientID);
void subscriberFilePop(std::string topicName, std::string clientID);
void subscriberFilePush(std::string topicName, std::string clientID);
int loadServer(std::string entity, std::map<std::string, std::vector<int>> &subscriberMap, std::map<std::string, Topic> &topicMap, std::map<std::string, int> &pubInts);
int loadClient(std::string entity, std::map<std::string, int> &nextTopicIDs);
int getNextPostID(std::string entityName, std::string topic);
int savePost(std::string entityName, std::string topic, std::string message, int postID);
void printTokens(std::vector<std::string> tokens);
std::vector <std::string> tokenize(char * input);

#endif