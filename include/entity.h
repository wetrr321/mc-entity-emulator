// ============================================================
// entity.h - 实体类声明
// 定义 Entity 类（珍珠/TNT等游戏实体）及其数据结构 EntityData
// EntityData 用于快照存储，Entity 继承自 EntityData
// ============================================================

#pragma once

#include <vector>
#include <cstring>

// TNT 引信倒计时长度（单位：游戏tick，1tick = 0.05秒）
#define TNT_PERIOD 80
// 实体状态：固定不动（被方块卡住等情况）
#define FIXED 0
// 实体状态：自由运动（空中/水中等）
#define FREE 1

// 前向声明，World 类定义在 world.h 中
class World;

// 实体数据快照结构体
// 用于序列化/拷贝实体状态，不包含行为逻辑
struct EntityData
{
    int id;           // 实体唯一ID
    int status;       // 实体状态：FIXED 或 FREE
    int count;        // 带数量的计数器（TNT为爆炸威力）
    char name[20];    // 实体名称："pearl" 或 "tnt"
    double x, y, z;   // 实体底部坐标（世界坐标系）
    double mx, my, mz;// 实体速度分量（米/tick）
    double com_y;     // 实体眼部相对底部的高度（用于爆炸计算）
    double g;         // 重力加速度（米/tick²）
    double drag;      // 空气阻力系数（每tick乘算）
    int tick = 0;     // 实体已存在的tick数
};

// 实体类，继承自 EntityData 以复用数据字段
// 同时封装实体的运动逻辑、爆炸受击逻辑
class Entity : private EntityData
{
private:
    World* my_world;  // 所属世界的指针，用于注册和交互
public:
    // 构造函数：创建新实体并注册到世界
    Entity(
        World* worldPtr_,
        const char* name_,
        int status_,
        int count_,
        double x_, double y_, double z_,
        double mx_, double my_, double mz_,
        double com_y_,
        double g_, double drag_
    ) ;

    // 拷贝构造函数：从另一个实体拷贝数据到指定世界
    Entity(World* world, const Entity& other);

    // 析构函数：销毁实体并输出日志
    ~Entity() ;

    // 输出实体详细信息到控制台
    void info() ;

    // 执行一个tick的物理运动（重力+阻力+速度更新）
    void nextMove();

    // 应用爆炸冲击力：根据爆心坐标和威力计算速度增量
    void applyExplosion(double x_, double y_, double z_, int power);

    // 设置TNT剩余引信时间（从当前tick回溯）
    void setRemainingTick(int tick_);

    // tick计数器+1
    void nextTick();

    // 获取实体已存活tick数
    int getTick();

    // 获取实体Y坐标（底部）
    double getY();

    // 获取实体X坐标
    double getX();

    // 获取实体Z坐标
    double getZ();

    // 获取X方向速度
    double getVX();

    // 获取Y方向速度
    double getVY();

    // 获取Z方向速度
    double getVZ();

    // 获取实体眼部相对底部高度
    double getComY();

    // 获取实体名称字符串
    char* getName();

    // 获取TNT剩余引信tick数（TNT_PERIOD - tick）
    int getRemainingTick();

    // 获取实体状态（FIXED/FREE）
    int getStatus();

    // 获取实体计数器值（TNT为爆炸威力）
    int getCount();

    // 获取实体唯一ID
    int getId();

    // 获取实体数据快照（用于序列化/拷贝）
    EntityData getData() const;
    // 设置实体计数器值（TNT为爆炸威力）
    void setCount(int count_);
    // 设置实体Y坐标（底部）
    void setY(double y_);
};
