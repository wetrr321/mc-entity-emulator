#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <stdexcept>  // out_of_range头文件
#include <iostream>  // cout头文件
#define ANY_TICK -1

class World;

enum class Status { NONE, JMP, JT, JF, CALL, RET, DO, SPAWN, QUIT};

struct Entry {

    int matchTick = -1;                             // -1=每tick匹配  >=0=仅该tick匹配
    std::function<void(World*)> acts;  // 动作列表（lambda）
    Status status = Status::JMP;                          // 做什么
    std::string target;                             // 跳转目标标签
    std::function<bool(int, World*)> cond;          // 条件函数（jt/jf 用）
};



class Executor {
public:
    World* world;
    std::vector<Entry> entries;
    std::map<std::string, int> labelMap;
public:
    Executor(World* world_, std::vector<Entry> entries_, std::map<std::string, int> labelMap_):
    world(world_),
    entries(entries_),
    labelMap(labelMap_){}
    ~Executor(){}

    void execute(){
        int line;

    try{
        line = labelMap.at("default");
    }
    catch(const std::out_of_range& e){
        std::cerr << "Error: In Init, Label Default have to be defined." << std::endl;
        return;
    }

    while(entries[line].status != Status::QUIT){
        if(entries[line].matchTick == ANY_TICK || entries[line].matchTick == world->getWorldTick())
        {}else{line++;continue;}
        switch(entries[line].status){
            case Status::NONE:
                line++;
                break;
            case Status::JMP:
                try{
                    line = labelMap.at(entries[line].target);
                }
                catch(const std::out_of_range& e){
                    std::cerr << "Error: In JMP, Label " << entries[line].target << " have to be defined." << std::endl;
                    return;
                }

                    break;
            case Status::DO:
                entries[line].acts(world);
                line++;
                break;
        }
        std::cout << "Line: " << line << std::endl;
        
    }
       
    }

};