// ============================================================
// entity.cpp - 实体类实现
// 包含 Entity 的构造函数、物理运动、爆炸受击等逻辑
// ============================================================

#include "entity.h"
#include "world.h"
#include <iostream>
#include <iomanip>
#include <cmath>

#define TNT_PERIOD 80
// 实体状态：固定不动（被方块卡住等情况）
#define FIXED 0
// 实体状态：自由运动（空中/水中等）
#define FREE 1

    // ============================================================
    // 构造函数：创建新实体
    // 初始化所有物理参数，自动注册到所属世界，分配唯一ID
    // ============================================================
    Entity::Entity(
        World* worldPtr_,
        const char* name_,
        int status_,
        int count_,
        double x_, double y_, double z_,
        double mx_, double my_, double mz_,
        double com_y_,
        double g_, double drag_
    ) :
    my_world(worldPtr_),
    EntityData{worldPtr_->registerId(),status_, count_, "", x_, y_, z_, mx_, my_, mz_, com_y_, g_, drag_}
    {
        strcpy(name, name_);
        if(my_world != nullptr)
            my_world->registerEntity(this);  // 自动注册到世界实体列表
    }

    // ============================================================
    // 拷贝构造函数：从另一个实体浅拷贝数据到新世界
    // 仅拷贝 EntityData 数据，不拷贝 my_world 指针
    // ============================================================
    Entity::Entity(World* world,const Entity& other):
    my_world(world),
    EntityData(other)
    {
    }

    // ============================================================
    // 析构函数：输出销毁日志
    // ============================================================
    Entity::~Entity() {
        std::cout << "destroy " << name << "\n";
    }

    // ============================================================
    // 输出实体完整信息到控制台（用于调试）
    // 包含：ID、名称、状态、坐标、速度、重力、阻力
    // ============================================================
    void Entity::info() {
        using namespace std;
        cout << fixed << setprecision(20);
        cout << right << setw(15) << "id:" << right << setw(25) << id << "\n";

        cout << right << setw(15) << "name:" << right << setw(25) << name << "\n";
        if(strcmp(name,"tnt")==0){
            cout << right << setw(15) << "power:" << right << setw(25) << count << "\n";
            cout << right << setw(15) << "countdown:" << right << setw(25) << getRemainingTick() << "\n";
        }
        else
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

    // ============================================================
    // 执行一个tick的物理运动
    // 珍珠：先减重力再乘阻力，再更新坐标（My改）
    // TNT： 先减重力更新坐标，再乘阻力（Java版TNT运动规则）
    // FIXED状态实体跳过运动
    // ============================================================
    void Entity::nextMove() {
        if(status == FIXED) return;
        if(strcmp(name, "pearl") == 0) {
            // 珍珠运动：速度 = (速度 - 重力) * 阻力
            my = (my - g) * drag;
            y += my;
            mx = mx * drag;
            x += mx;
            mz = mz * drag;
            z += mz;
        } else if(strcmp(name, "tnt") == 0) {
            // TNT运动：先减重力更新坐标，再乘阻力
            my -= g;
            y += my;
            my = my * drag;
            x += mx;
            mx = mx * drag;
            z += mz;
            mz = mz * drag;
        }
    }

    // ============================================================
    // 应用爆炸冲击力
    // 根据爆心坐标 (x_,y_,z_) 和爆炸威力 power 计算速度增量
    // 爆炸有效半径 = 8米，距离越近冲击力越大
    // 爆炸方向从爆心→实体眼部（不是底部）
    // ============================================================
    void Entity::applyExplosion(double x_, double y_, double z_,int power) {

        if(strcmp(name,"tnt")==0){
            // 爆心到实体底部的距离
            double d2 = std::sqrt((x-x_)*(x-x_)+(y-y_)*(y-y_)+(z-z_)*(z-z_));
            if(d2 >= 8) return;  // 超出爆炸半径，不受影响
            // 单位方向向量（从爆心指向实体眼部）
            double dy = (y-y_)/d2;
            double dx = (x-x_)/d2;
            double dz = (z-z_)/d2;
            // 冲击力 = max(0, 1-d2/8) * 方向 * 威力
            this->my += (std::max(0.0,1-d2/8))*dy*power;
            this->mx += (std::max(0.0,1-d2/8))*dx*power;
            this->mz += (std::max(0.0,1-d2/8))*dz*power;
            std::cout << "Explosion hit " << name <<" m change"<<(std::max(0.0,1-d2/8))<< ".\n";

        }else{
            // 爆心到实体底部的距离
            double d2 = std::sqrt((x-x_)*(x-x_)+(y-y_)*(y-y_)+(z-z_)*(z-z_));
            if(d2 >= 8) return;  // 超出爆炸半径，不受影响
            // 爆心到实体眼部的距离（用于方向计算）
            double d1 = std::sqrt((x-x_)*(x-x_)+(y+com_y-y_)*(y+com_y-y_)+(z-z_)*(z-z_));
            // 单位方向向量（从爆心指向实体眼部）
            double dy = (com_y+y-y_)/d1;
            double dx = (x-x_)/d1;
            double dz = (z-z_)/d1;
            // 冲击力 = max(0, 1-d2/8) * 方向 * 威力
            this->my += (std::max(0.0,1-d2/8))*dy*power;
            this->mx += (std::max(0.0,1-d2/8))*dx*power;
            this->mz += (std::max(0.0,1-d2/8))*dz*power;
            std::cout << "Explosion hit " << name <<" m change"<<(std::max(0.0,1-d2/8))<< ".\n";
        }

    }

    // ============================================================
    // 设置TNT剩余引信时间（从指定tick开始倒计时）
    // ============================================================
    void Entity::setRemainingTick(int tick_) {
        this->tick = std::max(0, TNT_PERIOD - tick_);
    }

    // ============================================================
    // tick计数器+1
    // ============================================================
    void Entity::nextTick() {
        this->tick++;
    }

    // ============================================================
    // 以下为各字段的 getter 方法
    // ============================================================
    int Entity::getTick() {
        return tick;
    }

    double Entity::getY() {
        return y;
    }

    double Entity::getX() {
        return x;
    }

    double Entity::getZ() {
        return z;
    }

    double Entity::getVX() {
        return mx;
    }

    double Entity::getVY() {
        return my;
    }

    double Entity::getVZ() {
        return mz;
    }

    double Entity::getComY() {
        return com_y;
    }

    char* Entity::getName() {
        return name;
    }

    int Entity::getRemainingTick() {
        return TNT_PERIOD - tick;
    }

    int Entity::getStatus() {
        return status;
    }

    int Entity::getCount() {
        return count;
    }

    int Entity::getId() {
        return id;
    }

    // 返回实体数据快照（向上转型为 EntityData）
    EntityData Entity::getData() const {
        return *this;
    };

    void Entity::setCount(int count_){
        count = count_;
    }
    void Entity::setY(double y_){
        y = y_;
    }