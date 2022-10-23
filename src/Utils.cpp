#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif 

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cstring>

#include "Topic.h"
#include "Message.h"

namespace fs = std::filesystem;

std::string STORAGE_DIR = "./Storage";

void sleepForMs(float miliseconds) {
    usleep(miliseconds*1000);
}

std::string subscriberFileRead(std::string topicName, std::string clientID){
    std::string subscribersDirectory = STORAGE_DIR + "/Subscribers/" + topicName + "/" + clientID;
    std::string contents;

    std::ifstream ifs(subscribersDirectory);
    std::string subFileContents;
    std::getline(ifs, contents);

    return contents;

}

void subscriberFilePop(std::string topicName, std::string clientID){
    return;
}

void subscriberFilePush(std::string topicName, std::string clientID ){
    return;
}

int getNextPostID(std::string entity, std::string topic){
    std::string topicDirectory = STORAGE_DIR + "/" + entity + "/" + topic + "/";

    int nextMessageID = 0;
    fs::directory_iterator it;
    try{
        fs::create_directory(topicDirectory);
        it = fs::directory_iterator(topicDirectory);
    }
    catch(const std::exception & e){
        std::cout << "Caught exception: " << e.what() << "\n";
    }

    for(const auto & entry : it){
        std::string pathString = entry.path().string();
        int entryID = stoi(pathString.substr(pathString.find_last_of("/\\") + 1));
        if(nextMessageID <= entryID){
            nextMessageID = entryID + 1;
        }
    }
    
    return nextMessageID;
}

int setupStorage(std::string entity){
    std::string storageDirectory = STORAGE_DIR + "/" + entity;
    if(fs::exists(storageDirectory)){
        return 1;
    }
    else{
        fs::create_directories(storageDirectory);
        if(entity == "Server"){
            std::string subscribersDirectory = STORAGE_DIR + "/Subscribers";
            // if the directory already exists, we try to recover from the crash
            if(!fs::exists(subscribersDirectory)){
                fs::create_directories(subscribersDirectory);
            }
            fs::create_directories(storageDirectory);
        }
        return 0;
    }
}

int savePost(std::string entity, std::string topic, std::string message, int postID){
    int nextPostID = getNextPostID(entity, topic);
    std::string path = STORAGE_DIR + "/" + entity + "/" + topic + "/";
    try{
        fs::create_directories(path);
        path = path + std::to_string(postID);
    }
    catch(const std::exception & e){
        std::cout << "Caught exception: " << e.what() << "\n";
        return 1;
    }

    std::ofstream stream(path);
    stream << message; 
    stream.close();
    return 0;
}

void printTokens(std::vector<std::string> tokens){
    for (int i = 0; i<tokens.size(); ++i)
        std::cout << "token " << i << ": " << tokens[i] << std::endl;
}

std::vector <std::string> tokenize(char * input){
    std::vector<std::string> res;
    char *token = std::strtok(input, " ");

    while (token != NULL){
        res.push_back(std::string(token));
        //std::cout << token << std::endl;
        token = strtok(NULL, " ");
    }
    return res;
}