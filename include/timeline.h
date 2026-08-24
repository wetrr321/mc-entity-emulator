#pragma once
#include "world.h"
#include "entry.h"
#include <vector>
#include <map>
#include <cmath>
#define ANY_TICK -1

class Timeline//用于注册事件与规定事件的发生条件
{
private:
    World* world;
    std::vector<Entry> entries;
    std::map<std::string, int> labelMap;
    Entry currentEntry;
public:
    Timeline(World* world_):
    world(world_){}
    ~Timeline(){}



    Timeline& Label(std::string label_){
        currentEntry.status = Status::NONE;
        labelMap[label_] = entries.size();
        entries.push_back(currentEntry);

        return *this;//声明作用域
    }

    Timeline& At(int tick){
        currentEntry.matchTick = tick;
        return *this;//声明程序作用时间
    }




    Timeline& Do(std::function<void(World*)> act){
        currentEntry.status = Status::DO;
        currentEntry.acts = act;
        entries.push_back(currentEntry);
        currentEntry.acts = nullptr; 
        return *this;
    }

    Timeline& Jmp(std::string target_){
        currentEntry.status = Status::JMP;
        currentEntry.target = target_;

        entries.push_back(currentEntry);
        return *this;//声明跳转
    }

    Timeline& Quit(){
        currentEntry.status = Status::QUIT;
        entries.push_back(currentEntry);
        return *this;
    }
    std::vector<Entry> getEntries(){
        return entries;
    }
    std::map<std::string, int> getLabelMap(){
        return labelMap;
    }
    World* getWorld(){
        return world;
    }

};