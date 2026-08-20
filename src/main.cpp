// ============================================================
// main.cpp - 程序主入口
// MC 空爆仿真工具：模拟末影珍珠 + TNT 的物理运动与爆炸
// 支持：空格单步 / Shift连跑 / Caps无限速 / Ctrl跟踪珍珠
// 右键旋转视角 / 中键平移 / 滚轮缩放 / 左键点击查看实体信息
// ============================================================
#include <iostream>
// 引入raylib基础库，窗口、绘制、3D功能
#include "raylib.h"
// raylib数学工具，Vector3向量、向量运算
#include "raymath.h"
// 我们自己写的实体头文件，Entity类、updateEntityList、常量定义都在这里
#include "entities/entity.h"
#include "entities/tnt.h"
#include "entities/pearl.h"
#include "world.h"
#include "raylib_tool.h"
// 存放Entity*指针的动态数组，用来管理全部实体
#include <vector>
// C字符串处理函数 strcmp 等
#include <cstring>
// 数学库 sqrt 等
#include <cmath>
// 读取json文件
#include "json.hpp"
// 目标渲染帧率，窗口每秒刷新多少次，不等于游戏tick
#define TARGET_FPS 60
// Shift按住时最大仿真速率（普通模式，每秒20tick）
#define SHIFT_SIM_TPS 20
// 每次tick之间的时间间隔（毫秒）
const float SHIFT_SIM_INTERVAL = 1000.0f / SHIFT_SIM_TPS;
// Caps切换：true=无限速全速仿真，不做时间节流
bool unlimitedSpeedMode = false;
// 模拟暂停开关
bool isSimulationPaused = false;
// ========== 全局世界对象 ==========
World world("default");     // 主世界
World copyWorld("copy");    // 备份世界（用于回溯）
int pearlId = -1;           // 追踪器ID指针
int tnt1;
/*
TODO:: 珍珠空爆模拟流程
1. 生成合适的珍珠与珍珠推进TNT
2. 在合适的tick生成空爆TNT与其推进TNT
3. 查看模拟结果
*/

bool isMeetCondition(){
    double dy=std::abs(world.getEntity(tnt1)->getY()+0.0612500011920928955078125-0.2125000059604644775390625-world.getEntity(pearlId)->getY());
    double moddy = std::fmod(dy,1.0);
    if(moddy < 1e-6&&dy < 50){
        std::cout<<"meet condition:"<<dy<<std::endl;
        return true;
    }
    return false;
}

void flow(){
    if(world.getWorldTick() == 0){
        // new Pearl(
        // &world,FREE,
        // 0,50,0,
        // 0,0,0
        // );
        for(int i=0;i<200;i++){
            new Tnt(
            &world,FREE,1,200-i,
            0,20,0,
            0,0.1*i,0
            );
        }

        copyWorld=world;
    }
    if(world.getWorldTick() == 200){world = copyWorld;}
    world.worldNextTick();
}
// ============================================================
// 主函数：程序入口
// ============================================================
int main()
{
    SimRender r(&world);  // 创建渲染器，绑定到主世界

    bool isTracking = false;        // 跟踪开关（镜头跟随珍珠）
    float shiftSimTimer = 0.0f;    // Shift连续仿真计时器（毫秒）
    // ------------------------------------------------------------------------

    while (!r.isDead())
    {
        float dt = r.getFrameTime();  // 获取当前帧耗时

        // ===== CapsLock：切换无限速模式 =====
        if(IsKeyPressed(KEY_CAPS_LOCK))
        {
            unlimitedSpeedMode = !unlimitedSpeedMode;
        }

        // ===== 按住 LeftShift：连续仿真 =====
        if(IsKeyDown(KEY_LEFT_SHIFT)||unlimitedSpeedMode)
        {
            if(unlimitedSpeedMode)
            {
                // 无限速模式：一帧内尽可能跑tick，不做时间节流
                for(int i=0;i<20;i++){
                    flow();
                } // 加安全上限，防止死循环卡死
            }
            else
            {
                // 普通限速模式：按固定TPS节流
                shiftSimTimer += dt * 1000.0f;
                while(shiftSimTimer >= SHIFT_SIM_INTERVAL)
                {
                    flow();
                    shiftSimTimer -= SHIFT_SIM_INTERVAL;
                }
            }
        }
        else
        {
            // 松开Shift重置计时器，防止积压
            shiftSimTimer = 0.0f;
        }

        Vector2 mouseDelta = GetMouseDelta();

        // ===== Ctrl：切换跟踪开关 =====
        if(IsKeyPressed(KEY_LEFT_CONTROL))
        {
            isTracking = !isTracking;
        }

        // ===== 空格：单次步进 =====
        if (IsKeyPressed(KEY_SPACE))
        {
            flow();
        }

        // ===== 跟踪逻辑：开启跟踪时，相机跟随第一个珍珠 =====
        if(isTracking)
        {
            Entity* targetPearl = nullptr;
            for(auto e : *world.getEntityListPtr())
            {
                if(strcmp(e->getName(), "pearl") == 0)
                {
                    targetPearl = e;
                    break;
                }
            }
            if(targetPearl != nullptr)
            {
                r.camFocus({(float)targetPearl->getX(), (float)targetPearl->getY(), (float)targetPearl->getZ()});
            }
        }

        // ===== 右键旋转视角（跟踪/非跟踪模式都可用） =====
        if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            r.camRotate(mouseDelta);
        }

        // ===== 中键平移（只有非跟踪模式才允许） =====
        if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) && !isTracking)
        {
            r.camPan(mouseDelta);
        }

        // ===== 滚轮缩放（跟踪模式依然允许） =====
        r.zoom(GetMouseWheelMove());
        r.camUpdate();
        r.renderFrame();

        // ===== 射线拾取，获取当前悬浮指向的实体 =====
        Entity* hoverEntity = r.getHoverEntity();

        // ===== 左键单击：输出实体详细信息到控制台 =====
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (hoverEntity != nullptr)
            {
                hoverEntity->info();  // 输出到终端
            }
        }

        // ===== 悬浮Tooltip窗口：鼠标旁边显示简要预览信息 =====
        if (hoverEntity != nullptr)
        {
            r.renderTooltip(hoverEntity);
        }

        // ===== UI 提示信息 =====
        const char* trackHint = isTracking ? "TRACKING PEARL [Ctrl to untrack]" : "SPACE=SingleTick | SHIFT=Run | Caps:Toggle UnlimitedSpeed | Ctrl:TrackPearl | RMB Rot | MMB Pan | Wheel Zoom";
        DrawText(trackHint, 10, 10, 8, RED);

        // 显示当前仿真模式
        if(unlimitedSpeedMode)
        {
            DrawText("MODE: UNLIMITED SPEED",10,24,12,ORANGE);
        }
        else
        {
            DrawText("MODE: LIMITED 20TPS",10,24,12,GREEN);
        }

        // ===== 显示世界信息和实体列表 =====
        int yText = 40;
        char buf[256]{};
        world.uiInfoSprintf(buf);
        DrawText(buf, 10, yText,16, BLACK);
        yText += 22;
        for (Entity* e : *world.getEntityListPtr())
        {
            e->uiInfoSprintf(buf);
            DrawText(buf, 10, yText,16, BLACK);
            yText += 22;
        }

        r.endRender();
    }


    CloseWindow();  // 关闭窗口
    return 0;
}