#include "entities/entity.h"
#include "raylib.h"
#include <iostream>
#include <iomanip>
class Tnt : public Entity
{
protected:
int power = 1;
int fuse = 80;
public:
Tnt (World* worldPtr_,//正常构造
    int status_,
    int power_,
    int fuse_,
    double x_, double y_, double z_,
    double mx_, double my_, double mz_):
    Entity(worldPtr_, "tnt", status_,
    x_, y_, z_,
    mx_, my_, mz_,
    0, 0.98f/16, 0,//眼部坐标
    0.98f, 0.98f, 0.98f,//碰撞箱
    0.04, 0.98),//y加速度 阻力
    power(power_),
    fuse(fuse_)
    {
        color = RED;
        worldPtr_->publishMessage("TNT created",WHITE);
    }
    Tnt (World* worldPtr_, const Tnt& other):
        Entity(worldPtr_, other)
    {
        power = other.power;
        fuse = other.fuse;
    }
public:
    Entity* clone(World* worldPtr) const override
    {   
        return new Tnt(worldPtr, *this);
    }
    void info() const override
    {
        using namespace std;
        cout << fixed << setprecision(20);
        cout << right << setw(15) << "id:" << right << setw(25) << id << "\n";
        cout << right << setw(15) << "name:" << right << setw(25) << name << "\n";
        cout << right << setw(15) << "tick:" << right << setw(25) << tick << "\n";
        cout << right << setw(15) << "status" << right << setw(25) << (status==FIXED?"FIXED":"FREE") << "\n";
        cout << right << setw(15) << "power:" << right << setw(25) << power << "\n";
        cout << right << setw(15) << "fuse:" << right << setw(25) << fuse << "\n";
        cout << right << setw(15) << "x:"    << right << setw(25) << x << "\n";
        cout << right << setw(15) << "y:"    << right << setw(25) << y << "\n";
        cout << right << setw(15) << "z:"    << right << setw(25) << z << "\n";
        cout << right << setw(15) << "mx:"   << right << setw(25) << mx << "\n";
        cout << right << setw(15) << "my:"   << right << setw(25) << my << "\n";
        cout << right << setw(15) << "mz:"   << right << setw(25) << mz << "\n";
        cout << right << setw(15) << "g:"    << right << setw(25) << g << "\n";
        cout << right << setw(15) << "drag:" << right << setw(25) << drag << "\n";
        cout << right << setw(15) << "bounding:" << right << setw(25) << bounding_x << "\n";
        cout << "----------------------------------------\n";
    }
    void nextMove() override
    {
        // TNT运动：先减重力更新坐标，再乘阻力
        my -= g;
        y += my;
        my = my * drag;
        x += mx;
        mx = mx * drag;
        z += mz;
        mz = mz * drag;
    }
    void applyExplosion(double x_, double y_, double z_, int power) override
    {
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
        my_world->publishMessage("Explosion hit "+std::string(name)+" id: "+std::to_string(id)+" m change "+std::to_string(std::max(0.0,1-d2/8)),WHITE);
    }
    void uiInfoSprintf(char* buf) const override {sprintf(buf,"id: %d, name: %s, tick: %d, power: %d, fuse: %d", id, name, tick, power, fuse);}
    void nextTick() override
    {
        fuse--;
        tick++;
    }
    int getFuse() const {return fuse;}
    int getPower() const {return power;}
};