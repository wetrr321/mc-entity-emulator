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
#include "world.h"
#include "raylib.h"
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
    ) : 
    my_world(worldPtr_),
    id(worldPtr_->registerId()),
    status(status_),
    x(x_),
    y(y_),
    z(z_),
    mx(mx_),
    my(my_),
    mz(mz_),
    com_x(com_x_),
    com_y(com_y_),
    com_z(com_z_),
    bounding_x(bounding_x_),
    bounding_y(bounding_y_),
    bounding_z(bounding_z_),
    g(g_),
    drag(drag_)
    {
        std::strcpy(name, name_);
        if(my_world != nullptr)
            my_world->registerEntity(this);  // 自动注册到世界实体列表
    }
    Entity(World* worldPtr_, const Entity& other):
        my_world(worldPtr_)
    {
        std::strcpy(name, other.name);
        id = other.id;
        status = other.status;
        x = other.x;
        y = other.y;
        z = other.z;
        mx = other.mx;
        my = other.my;
        mz = other.mz;
        com_x = other.com_x;
        com_y = other.com_y;
        com_z = other.com_z;
        bounding_x = other.bounding_x;
        bounding_y = other.bounding_y;
        bounding_z = other.bounding_z;
        g = other.g;
        drag = other.drag;
        tick = other.tick;
        color = other.color;
    }
    virtual Entity* clone(World* worldPtr) const = 0;

    // ============================================================
    // 析构函数：输出销毁日志
    // ============================================================
    virtual ~Entity() {
        my_world->publishMessage("Entity destroied: "+std::string(name)+" id: "+std::to_string(id),RED);
    }
public:
    // ============================================================
    // 输出实体完整信息到控制台（用于调试）
    // 包含：ID、名称、状态、坐标、速度、重力、阻力
    // ============================================================
    virtual void info() const {
        using namespace std;
        cout << fixed << setprecision(20);
        cout << right << setw(15) << "id:" << right << setw(25) << id << "\n";
        cout << right << setw(15) << "name:" << right << setw(25) << name << "\n";
        cout << right << setw(15) << "tick:" << right << setw(25) << tick << "\n";
        cout << right << setw(15) << "status" << right << setw(25) << (status==FIXED?"FIXED":"FREE") << "\n";
        cout << right << setw(15) << "x:"    << right << setw(25) << x << "\n";
        cout << right << setw(15) << "y:"    << right << setw(25) << y << "\n";
        cout << right << setw(15) << "z:"    << right << setw(25) << z << "\n";
        cout << right << setw(15) << "mx:"   << right << setw(25) << mx << "\n";
        cout << right << setw(15) << "my:"   << right << setw(25) << my << "\n";
        cout << right << setw(15) << "mz:"   << right << setw(25) << mz << "\n";
        cout << right << setw(15) << "g:"    << right << setw(25) << g << "\n";
        cout << right << setw(15) << "drag:" << right << setw(25) << drag << "\n";
        cout << "----------------------------------------\n";
    }
    virtual void uiInfoSprintf(char* buf) const {sprintf(buf,"id: %d, name: %s, tick: %d", id, name, tick);};
    // 执行一个tick的物理运动（重力+阻力+速度更新）
    virtual void nextMove(){
        std::cout<<"Raw Entity Move"<<name<<std::endl;
    };

    // 应用爆炸冲击力：根据爆心坐标和威力计算速度增量
    virtual void applyExplosion(double x_, double y_, double z_, int power) {std::cout<<"Apply Explosion on Raw Entity"<<std::endl;}
    // tick计数器+1
    virtual void nextTick(){tick++;};

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
