#include "entities/entity.h"
#include <iostream>
#include <iomanip>
class Pearl : public Entity
{
protected:

public:
Pearl (World* worldPtr_,//正常构造
    int status_,
    double x_, double y_, double z_,
    double mx_, double my_, double mz_):
    Entity(worldPtr_, "pearl", status_,
    x_, y_, z_,
    mx_, my_, mz_,
    0, 0.85f*0.25, 0,//眼部坐标
    0.25, 0.25, 0.25,//碰撞箱尺寸
    0.03, 0.99f)//y加速度 阻力
    {
        color={24, 62, 12, 200};
        worldPtr_->publishMessage("Pearl created",WHITE);
    }

Pearl (World* worldPtr_, const Pearl& other):
    Entity(worldPtr_, other)
{

}
public:
    Entity* clone(World* worldPtr) const override
    {
        return new Pearl(worldPtr, *this);
    }
    void info() const override
    {
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
    void nextMove() override
    {
        // 珍珠运动：速度 = (速度 - 重力) * 阻力
        my = (my - g) * drag;
        y += my;
        mx = mx * drag;
        x += mx;
        mz = mz * drag;
        z += mz;
    }
    void applyExplosion(double x_, double y_, double z_, int power) override
    {
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
        my_world->publishMessage("Explosion hit "+std::string(name)+" id: "+std::to_string(id)+" m change "+std::to_string(std::max(0.0,1-d2/8)),WHITE);
    }
    void uiInfoSprintf(char* buf) const override {sprintf(buf,"id: %d, name: %s, tick: %d", id, name, tick);}
    void nextTick() override
    {
        tick++;
    }
};