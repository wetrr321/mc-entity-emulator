// ============================================================
// world.cpp - 世界类实现
// 管理实体列表的生命周期、tick更新循环、爆炸事件分发
// ============================================================

#include "world.h"
#include "message.h"
#include "entities/entity.h"
#include <cstring>
#include <iostream>
#define MESSAGE_MAX 40
#define MESSAGE_LIFE 2
// ============================================================
// 构造函数：初始化世界名称和计数器
// ============================================================
World::World(const char* name_) 
{
    strcpy(name, name_);
    id = 0;
    worldTick = 0;
}

// ============================================================
// 拷贝构造函数：深拷贝所有实体到新世界
// 每个实体通过 Entity 的拷贝构造函数在新世界中重建
// ============================================================
World::World(const World& other) :
    id(other.id),
    worldTick(other.worldTick)
{
    for(auto i = other.entityList.begin(); i != other.entityList.end(); i++) {
        entityList.push_back((*i)->clone(this));
    }
}

// ============================================================
// 赋值运算符：先销毁自身所有实体，再深拷贝源世界的实体
// ============================================================
World& World::operator=(const World& other) {
    if(this == &other) return *this;  // 防止自赋值
    this->publishMessage("Rewind World!",YELLOW);
    // 销毁自己的实体列表
    for(auto e : entityList) {
        delete e;
    }
    entityList.clear();
    // 拷贝世界数据（tick、ID计数）
    id = other.id;
    worldTick = other.worldTick;
    // 深拷贝所有实体到当前世界
    for(auto i = other.entityList.begin(); i != other.entityList.end(); i++) {
        entityList.push_back((*i)->clone(this));
    }
    return *this;
}

// ============================================================
// 析构函数：销毁所有实体并释放内存
// ============================================================
World::~World() {
    for(auto e : entityList) {
        delete e;
    }
    entityList.clear();
}



// ============================================================
// 更新所有实体：一个完整的游戏tick
// 1. 每个实体 tick+1
// 2. 每个实体执行物理运动
// 3. 检查TNT是否引信归零 → 爆炸 → 销毁TNT
// ============================================================
void World::updateEntityList()
{
    for(auto i = entityList.begin(); i != entityList.end(); )
    {
        (*i)->nextTick();
        (*i)->nextMove();
        if((*i)->shouldExpode())
        {
            // 爆炸，遍历全部实体施加冲击力
            for(auto j = entityList.begin(); j != entityList.end(); ++j)
            {
                if(i == j) continue;
                (*j)->applyExplosion(
                    (*i)->getX(),
                    (*i)->getComY() + (*i)->getY(),
                    (*i)->getZ(),
                    (*i)->getExplosionPower()
                );
            }
            // ✅只有倒计时到0才销毁
            delete *i;
            i = entityList.erase(i);
            continue; // 直接跳到下一轮for，不走下面 ++i
        }else{
            ++i;
        }

    }
}

// ============================================================
// 输出所有实体信息到控制台
// ============================================================
void World::printEntityInfo() {
    for(auto i = entityList.begin(); i != entityList.end(); i++) {
        (*i)->info();
    }
}

// ============================================================
// 获取实体：根据实体ID返回实体指针
// ============================================================
Entity* World::getEntity(int id_){
        for(auto e : entityList) {
            if(e->getId() == id_) return e;
        }
        return nullptr;
    }

void World::registerEntity(Entity* e) {
    entityList.push_back(e);
}
void World::worldNextTick(){
    tickGrow();
    updateEntityList();
    updateMessageStack();
}

void World::uiInfoSprintf(char* buf)
{
    sprintf(buf,"worldTick:%d",worldTick);
}

void World::publishMessage(std::string msg,Color col) {
    Message m;
    m.msg = msg;
    m.color = col;
    m.birthTick = worldTick;
    messageStack.push_back(m);
    if(messageStack.size() > MESSAGE_MAX) {
        messageStack.erase(messageStack.begin());
    }
}

void World::updateMessageStack() {
    for(auto i = messageStack.begin(); i != messageStack.end();) {
        if(worldTick - i->birthTick >= MESSAGE_LIFE) {
            i = messageStack.erase(i);
        }else{
            i++;
        }
    }
}



