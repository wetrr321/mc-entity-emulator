// ============================================================
// entity.h - 实体类声明
// 定义 Entity 类（珍珠/TNT等游戏实体）及其数据结构 EntityData
// EntityData 用于快照存储，Entity 继承自 EntityData
// ============================================================

#pragma once

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <raylib.h>
#include "world.h"
// TNT 引信倒计时长度（单位：游戏tick，1tick = 0.05秒）
#define TNT_PERIOD 80
// 实体状态：固定不动（被方块卡住等情况）
#define FIXED 0
// 实体状态：自由运动（空中/水中等）
#define FREE 1


class World;


// 实体类，继承自 EntityData 以复用数据字段
// 同时封装实体的运动逻辑、爆炸受击逻辑
class Entity 
{
protected:
    Color color=WHITE;
    World* my_world;  // 所属世界的指针，用于注册和交互
    int id;           // 实体唯一ID
    int status;       // 实体状态：FIXED 或 FREE
    char name[20];    // 实体名称："pearl" 或 "tnt"
    double x, y, z;   // 实体底部坐标（世界坐标系）
    double mx, my, mz;// 实体速度分量（米/tick）
    double com_x,com_y,com_z;     // 实体眼部相对底部的高度（用于爆炸计算）
    double bounding_x,bounding_y,bounding_z;     // 实体眼部相对底部的高度（用于爆炸计算）
    double g;         // 重力加速度
    double drag;      // 空气阻力系数（每tick乘算）
    int tick = 0;     // 实体已存在的tick数
public:
    // 构造函数：创建新实体并注册到世界
    // ============================================================
    // 构造函数：创建新实体
    // 初始化所有物理参数，自动注册到所属世界，分配唯一ID
    // ============================================================
    Entity(
        World* worldPtr_,
        const char* name_,
        int status_,

        double x_, double y_, double z_,
        double mx_, double my_, double mz_,
        double com_x_, double com_y_,double com_z_,
        double bounding_x_, double bounding_y_, double bounding_z_,
        double g_, double drag_
    ) ;
    Entity(World* worldPtr_, const Entity& other);
    virtual Entity* clone(World* worldPtr) const = 0;

    // ============================================================
    // 析构函数：输出销毁日志
    // ============================================================
    virtual ~Entity();
public:
    // ============================================================
    // 输出实体完整信息到控制台（用于调试）
    // 包含：ID、名称、状态、坐标、速度、重力、阻力
    // ============================================================
    virtual void info() const ;
    virtual void uiInfoSprintf(char* buf) const;
    // 执行一个tick的物理运动（重力+阻力+速度更新）
    virtual void nextMove();

    // 应用爆炸冲击力：根据爆心坐标和威力计算速度增量
    virtual void applyExplosion(double x_, double y_, double z_, int power) ;
    
    
    
    
    
    // tick计数器+1
    virtual void nextTick(){tick++;};

    virtual bool shouldExpode() {return false;}
    virtual int  getExplosionPower() const { return 0; }
    // 获取实体已存活tick数
    int getTick() {return tick;};

    // 获取实体Y坐标（底部）
    double getY() {return y;};

    // 获取实体X坐标
    double getX() {return x;};

    // 获取实体Z坐标
    double getZ() {return z;};

    // 获取X方向速度
    double getVX() {return mx;};

    // 获取Y方向速度
    double getVY() {return my;};

    // 获取Z方向速度
    double getVZ() {return mz;};

    // 获取实体眼部相对底部高度
    double getComX() {return com_x;};
    double getComY() {return com_y;};
    double getComZ() {return com_z;};
    // 获取实体边界高度
    double getBoundingX() {return bounding_x;};
    double getBoundingY() {return bounding_y;};
    double getBoundingZ() {return bounding_z;};

    // 获取实体名称字符串
    const char* getName() {return name;};

    // 获取实体状态（FIXED/FREE）
    int getStatus() {return status;};

    // 获取实体唯一ID
    int getId() {return id;};

    Color getColor() {return color;};


};
