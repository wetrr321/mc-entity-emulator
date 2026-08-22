// ============================================================
// world.h - 世界类声明
// 管理所有实体、世界tick、实体ID注册
// 负责每tick更新所有实体并处理TNT爆炸事件
// ============================================================

#pragma once

#include "message.h"

#include "string"

#include <vector>

class Entity;



// 世界类，继承自 WorldData 复用数据字段
// 管理实体列表、tick更新、爆炸事件分发
class World 
{
private:
    std::vector<Entity*> entityList;  // 实体指针列表（World负责释放内存）
    std::vector<Message> messageStack; // 消息stack（World负责释放内存）
    int worldTick = 0;  // 世界已运行的tick总数
    int id = 0;         // 当前实体ID分配计数器
    char name[20];                     // 世界名称
public:
    // 构造函数：创建指定名称的世界
    World(const char* name_);

    // 拷贝构造函数：深拷贝所有实体到新世界
    World(const World& other);

    // 赋值运算符：销毁旧实体，深拷贝新实体
    World& operator=(const World& other);

    // 析构函数：销毁所有实体并释放内存
    ~World();


    // 更新所有实体：tick+1、移动、检查TNT爆炸
    void updateEntityList();

    // 输出所有实体信息到控制台
    void printEntityInfo();

    Entity* getEntity(int id_) ;
    // 世界tick+1
    void tickGrow(){worldTick++;}
    // 获取当前世界tick数
    int getWorldTick() { return worldTick;}

    int registerId() {
        id++;
        return id;
    }

    const std::vector<Entity*>& getEntityListPtr() {return entityList;}
    const std::vector<Message>& getMessageStack() {return messageStack;}



    void registerEntity(Entity* e);

    void worldNextTick();

    void uiInfoSprintf(char* buf);

    void publishMessage(std::string msg,Color col) ;

    void updateMessageStack();

};