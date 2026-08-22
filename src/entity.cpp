// ============================================================
// entity.h - 实体类声明
// 定义 Entity 类（珍珠/TNT等游戏实体）及其数据结构 EntityData
// EntityData 用于快照存储，Entity 继承自 EntityData
// ============================================================



#include "entities/entity.h"
#include "world.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>


// TNT 引信倒计时长度（单位：游戏tick，1tick = 0.05秒）
#define TNT_PERIOD 80
// 实体状态：固定不动（被方块卡住等情况）
#define FIXED 0
// 实体状态：自由运动（空中/水中等）
#define FREE 1

    Entity::Entity(
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
    Entity::Entity(World* worldPtr_, const Entity& other):
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

    // ============================================================
    // 析构函数：输出销毁日志
    // ============================================================
    Entity::~Entity() {
        my_world->publishMessage("Entity destroied: "+std::string(name)+" id: "+std::to_string(id),RED);
    }

    // ============================================================
    // 输出实体完整信息到控制台（用于调试）
    // 包含：ID、名称、状态、坐标、速度、重力、阻力
    // ============================================================
    void Entity::info() const {
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
    void Entity::uiInfoSprintf(char* buf) const {sprintf(buf,"id: %d, name: %s, tick: %d", id, name, tick);};
    // 执行一个tick的物理运动（重力+阻力+速度更新）
    void Entity::nextMove(){
        my_world->publishMessage("Entity move: "+std::string(name)+" id: "+std::to_string(id),RED);
    };

    // 应用爆炸冲击力：根据爆心坐标和威力计算速度增量
    void Entity::applyExplosion(double x_, double y_, double z_, int power) {
        my_world->publishMessage("Entity apply explosion: "+std::string(name)+" id: "+std::to_string(id),RED);
    }




