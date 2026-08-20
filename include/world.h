// ============================================================
// world.h - 世界类声明
// 管理所有实体、世界tick、实体ID注册
// 负责每tick更新所有实体并处理TNT爆炸事件
// ============================================================

#pragma once
#include <vector>
#include "entity.h"

// 世界数据快照结构体
struct WorldData{
    int worldTick = 0;  // 世界已运行的tick总数
    int id = 0;         // 当前实体ID分配计数器
};

// 世界类，继承自 WorldData 复用数据字段
// 管理实体列表、tick更新、爆炸事件分发
class World : public WorldData
{
private:
    std::vector<Entity*> entityList;  // 实体指针列表（World负责释放内存）
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

    // 生成实体并返回其ID（实体必须用new创建，World接管生命周期）
    int spawnEntity(Entity* e);

    // 更新所有实体：tick+1、移动、检查TNT爆炸
    void updateEntityList();

    // 输出所有实体信息到控制台
    void printEntityInfo();



    
    Entity* World::getEntity(int id_) {
        for(auto e : entityList) {
            if(e->getId() == id_) return e;
        }
        return nullptr;
    }
    // 世界tick+1
    void World::tickGrow(){
        worldTick++;
    }
    // 获取当前世界tick数
    int getWorldTick() {
        return worldTick;
    }

    int registerId() {
        id++;
        return id;
    }
    // ============================================================
    // 获取实体指针列表的引用（渲染器遍历用）
    // ============================================================
    std::vector<Entity*>* getEntityListPtr() {
        return &entityList;
    }
    // ============================================================
    // 将实体指针加入列表（由 Entity 构造函数自动调用）
    // ============================================================
    void registerEntity(Entity* e) {
        entityList.push_back(e);
    }
    // ============================================================
    // 推进一个完整的世界tick：tick+1 → 更新所有实体
    // ============================================================
    void worldNextTick(){
        tickGrow();
        updateEntityList();
    }
};