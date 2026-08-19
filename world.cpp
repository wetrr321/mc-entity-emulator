// ============================================================
// world.cpp - 世界类实现
// 管理实体列表的生命周期、tick更新循环、爆炸事件分发
// ============================================================

#include "world.h"
#include "entity.h"
#include <iostream>

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
    WorldData(other)
    {
        for(auto i = other.entityList.begin(); i != other.entityList.end(); i++) {
            entityList.push_back(new Entity(this,**i));
        }
    }

    // ============================================================
    // 赋值运算符：先销毁自身所有实体，再深拷贝源世界的实体
    // ============================================================
    World& World::operator=(const World& other) {
        if(this == &other) return *this;  // 防止自赋值
        std::cout << "Copy World!" << std::endl;
        // 销毁自己的实体列表
        for(auto e : entityList) {
            delete e;
        }
        entityList.clear();
        // 拷贝世界数据（tick、ID计数）
        WorldData::operator=(other);
        // 深拷贝所有实体到当前世界
        for(auto i = other.entityList.begin(); i != other.entityList.end(); i++) {
            entityList.push_back(new Entity(this,**i));
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
    // 生成实体：实体必须用 new 创建传入，World接管其生命周期
    // 返回实体ID，若传入 nullptr 则返回 -1
    // ============================================================
    int World::spawnEntity(Entity* e) {
        if(e == nullptr) return -1;
        return e->getId();
    }

    // ============================================================
    // 更新所有实体：一个完整的游戏tick
    // 1. 每个实体 tick+1
    // 2. 每个实体执行物理运动
    // 3. 检查TNT是否引信归零 → 爆炸 → 销毁TNT
    // ============================================================
    void World::updateEntityList() {
        for(auto i = entityList.begin(); i != entityList.end(); ) {
            (*i)->nextTick();     // tick计数器+1
            (*i)->nextMove();     // 执行物理运动

            // TNT引信归零：触发爆炸，销毁TNT
            if((*i)->getRemainingTick() <= 0 && strcmp((*i)->getName(), "tnt") == 0) {
                // 对除自己外的所有实体施加爆炸冲击
                for(auto j = entityList.begin(); j != entityList.end(); j++) {
                    if(i == j) continue;
                    (*j)->applyExplosion((*i)->getX(), (*i)->getComY()+(*i)->getY(), (*i)->getZ(),(*i)->getCount());
                    
                }
                delete (*i);                    // 销毁TNT实体
                i = entityList.erase(i);        // 从列表中移除，erase返回下一个迭代器
                
            }else{
                i++;  // 未爆炸，正常前进
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
    // 根据ID查找实体，找不到返回 nullptr
    // ============================================================
    Entity* World::getEntity(int id_) {
        for(auto e : entityList) {
            if(e->getId() == id_) return e;
        }
        return nullptr;
    }

    // ============================================================
    // 世界tick计数器+1
    // ============================================================
    void World::tickGrow(){
        worldTick++;
    }

    // ============================================================
    // 获取当前世界tick数
    // ============================================================
    int World::getWorldTick() {
        return worldTick;
    }

    // ============================================================
    // 分配新实体ID（自增计数器）
    // ============================================================
    int World::registerId() {
        id++;
        return id;
    }

    // ============================================================
    // 获取所有实体数据的快照列表（用于存档/拷贝世界状态）
    // ============================================================
    std::vector<EntityData> World::getEntityListData() const {
        std::vector<EntityData> entityListData;
        for(auto e : entityList) {
            entityListData.push_back(e->getData());
        }
        return entityListData;
    }

    // ============================================================
    // 获取实体指针列表的引用（渲染器遍历用）
    // ============================================================
    std::vector<Entity*>* World::getEntityListPtr() {
        return &entityList;
    }

    // ============================================================
    // 将实体指针加入列表（由 Entity 构造函数自动调用）
    // ============================================================
    void World::registerEntity(Entity* e) {
        entityList.push_back(e);
    }

    // ============================================================
    // 推进一个完整的世界tick：tick+1 → 更新所有实体
    // ============================================================
    void World::worldNextTick(){
        tickGrow();
        updateEntityList();
    }