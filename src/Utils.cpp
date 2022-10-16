#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

std::string STORAGE_DIR = "./Storage";

void setupStorage(std::string entity){
    std::string directories = STORAGE_DIR + "/" + entity;
    std::filesystem::create_directories(directories);
}

int getNextPostID(std::string entityName, std::string topic){
    std::string topicDirectory = STORAGE_DIR + "/" + entityName + "/" + topic + "/";
    int nextMessageID = 0;
    std::filesystem::directory_iterator it;
    try{
        std::filesystem::create_directory(topicDirectory);
        it = std::filesystem::directory_iterator(topicDirectory);
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

int savePost(std::string entityName, std::string topic, std::string message, int postID){
    int nextPostID = getNextPostID(entityName, topic);
    std::string path = STORAGE_DIR + "/" + entityName + "/" + topic + "/";
    try{
        std::filesystem::create_directories(path);
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