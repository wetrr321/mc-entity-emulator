// ============================================================
// raylib_tool.h - 仿真渲染器类声明
// 封装 raylib 3D 渲染功能，管理相机、实体绘制、鼠标交互
// 使用球面轨道相机模型，支持旋转/平移/缩放/射线拾取
// ============================================================

#pragma once
#include "raylib.h"
#include "world.h"
#include "entities/entity.h"
#include "message.h"
#include "raymath.h"

// 仿真渲染器类
// 负责：窗口管理、3D相机控制、实体渲染、速度箭头、鼠标拾取浮窗
class SimRender{
private:
    Camera3D cam;           // raylib 3D 相机结构体

    World* world;           // 指向当前仿真世界的指针

    // ===== 球面轨道相机模型参数 =====
    float camYaw   = 45.0f;     // 水平偏航角（度）
    float camPitch = 30.0f;     // 垂直俯仰角（度）
    Vector3 camTarget{0.0f, 50.0f, 0.0f};  // 相机注视目标点
    Vector3 camPosition{};                  // 相机位置（由球面公式计算）
    const float rotSpeed = 0.25f;           // 鼠标旋转灵敏度
    const float panSpeed = 0.0005f;         // 鼠标平移灵敏度
    float camOrbitalDistance = 100.0f;      // 相机到目标的轨道半径（正交模式下=fovy）

    int width,height;   // 窗口宽度、高度

    bool isTracking = false;       // 是否正在跟踪实体标志
    int trackingId = -1;           // 追踪器ID指针


    bool unlimitedSpeedMode = false;  // 无限速模拟开关

    // 初始化相机默认参数
    void initCamera();

    // 根据球面坐标公式（yaw/pitch/distance）重新计算相机位置
    void recomputeCameraSpherical();
public:
    // 构造函数：创建窗口并初始化相机
    SimRender(World* world_, int width_, int height_, int target_fps_);

    // 设置相机注视目标（用于跟踪实体）
    void camFocus(Vector3 targetPos);

    // 鼠标拖动旋转相机视角（修改yaw/pitch）
    void camRotate(Vector2 mouseDelta);

    // 鼠标拖动平移相机注视点
    void camPan(Vector2 mouseDelta);

    // 滚轮缩放（修改正交视口高度）
    void zoom(float wheel);

    // 将模型参数同步写入 raylib 相机结构体（每帧调用）
    void camUpdate();

    // 获取实体底部中心坐标（double→float转换）
    Vector3 getFloatCenter(Entity* e);

    // 获取实体速度向量（double→float转换）
    Vector3 getFloatVel(Entity* e);

    // 渲染单个实体：TNT为红色立方体，珍珠为深绿色小方块
    void renderEntity(Entity* e);

    // 渲染实体速度箭头（黄色圆柱+圆锥）
    void renderVel(Entity* e);

    // 渲染一整帧：清屏→3D模式→网格→实体→速度箭头
    void renderFrame();

    // 渲染鼠标悬浮提示框（实体名称、tick、Y坐标、速度）
    void renderTooltip(Entity* e);

    // 射线拾取：获取鼠标悬浮的最近实体
    Entity* getHoverEntity();

    // 结束当前帧绘制（EndDrawing）
    void endRender();

    // 检查窗口是否关闭
    bool isDead();

    // 获取当前帧耗时（秒）
    float getFrameTime();
    // 渲染世界原点XYZ调试坐标轴
    void SimDrawWorldAxis(Vector3 origin, float axisLen);
    // 放在 EndMode3D(); 之后
    void drawLogUi()
    {
        const auto& logs = world->getMessageStack();
        int y = 10;
        DrawText("Misc. output", width - 300, y, 24, GREEN);
        y += 24;
        for(const auto& msg : logs)
        {
            DrawText(msg.msg.c_str(), width - 300, y, 14, msg.color);
            y += 18;
        }
    }
    void drawEntityLogUi(){
        int yText = 50;
        char buf[256]{};
        world->uiInfoSprintf(buf);
        DrawText(buf, 10, yText,16, BLACK);
        yText += 22;
        for (Entity* e : world->getEntityListPtr())
        {
            if(yText > height){break;}
            e->uiInfoSprintf(buf);
            DrawText(buf, 10, yText,16, BLACK);
            yText += 22;
        }
    }

    bool getIsTracking()const{
        return isTracking;
    }
    void setIsTracking(bool isTracking_){
        isTracking = isTracking_;
    }
    void setTrackingId(int trackingId_){
        trackingId = trackingId_;
    }
    int getTrackingId()const{
        return trackingId;
    }
};