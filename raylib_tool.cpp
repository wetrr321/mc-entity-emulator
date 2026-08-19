// ============================================================
// raylib_tool.cpp - 仿真渲染器实现
// 包含 SimRender 类的全部方法实现：
// 球面轨道相机、实体渲染、速度箭头、射线拾取、悬浮提示
// ============================================================

// 引入raylib基础库，窗口、绘制、3D功能
#include "raylib.h"
// raylib数学工具，Vector3向量、向量运算
#include "raymath.h"
// 我们自己写的实体头文件，Entity类、updateEntityList、常量定义都在这里
#include "entity.h"
#include "world.h"
#include "raylib_tool.h"
// 存放Entity*指针的动态数组，用来管理全部实体
#include <vector>
// C字符串处理函数 strcmp 等
#include <cstring>
// 数学库 sqrt 等
#include <cmath>
// 控制台输出
#include <iostream>
// 默认渲染帧率
#define DEFAULT_FPS 60.0f


// ============================================================
// 初始化相机默认参数
// 使用正交投影，fovy 同时作为轨道距离
// ============================================================
void SimRender::initCamera(){
    cam = {0};
    cam.up         = {0.0f, 1.0f, 0.0f};
    cam.projection = CAMERA_ORTHOGRAPHIC;  // 正交投影，适合工程可视化
    cam.fovy       = camOrbitalDistance;   // 正交模式下fovy=视口高度
}

// ============================================================
// 根据球面坐标公式重新计算相机位置
// 输入：camYaw（偏航角）、camPitch（俯仰角）、camOrbitalDistance（轨道半径）
// 输出：camPosition（相机世界坐标）
// 公式：x = R*cos(pitch)*sin(yaw), y = R*sin(pitch), z = R*cos(pitch)*cos(yaw)
// ============================================================
void SimRender::recomputeCameraSpherical()
{
    float yawRad   = DEG2RAD * camYaw;
    float pitchRad = DEG2RAD * camPitch;
    camPosition.x = camTarget.x + camOrbitalDistance * cosf(pitchRad) * sinf(yawRad);
    camPosition.y = camTarget.y + camOrbitalDistance * sinf(pitchRad);
    camPosition.z = camTarget.z + camOrbitalDistance * cosf(pitchRad) * cosf(yawRad);
}

// ============================================================
// 构造函数：创建窗口并初始化相机
// ============================================================
SimRender::SimRender(World* world_):
world(world_)
{
    InitWindow(600, 1000, "MC空爆仿真");
    SetTargetFPS(DEFAULT_FPS);
    initCamera();
}

// ============================================================
// 设置相机注视目标（用于跟踪实体）
// ============================================================
void SimRender::camFocus(Vector3 targetPos){
    camTarget = targetPos;
}

// ============================================================
// 鼠标拖动旋转相机视角
// 水平拖动→修改yaw（偏航角）
// 垂直拖动→修改pitch（俯仰角），限制在±89.99°避免万向节锁
// ============================================================
void SimRender::camRotate(Vector2 mouseDelta){
    camYaw   -= mouseDelta.x * rotSpeed;
    camPitch -= mouseDelta.y * rotSpeed;
    camPitch = Clamp(camPitch, -89.99f, 89.99f);  // 避免万向锁
}

// ============================================================
// 鼠标拖动平移相机注视点
// 计算相机右向量和上向量，沿屏幕方向平移目标点
// 平移速度与轨道距离成正比（远则快，近则慢）
// ============================================================
void SimRender::camPan(Vector2 mouseDelta){
    // 相机右向量 = (目标-位置) × 上向量 的归一化
    Vector3 camRight = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(camTarget, camPosition), cam.up));
    Vector3 camUpDir = cam.up;
    float scaleFactor = 2.5;
    // 沿右向量平移（鼠标水平移动）
    camTarget = Vector3Add(camTarget, Vector3Scale(camRight, -mouseDelta.x * panSpeed * camOrbitalDistance * scaleFactor));
    // 沿上向量平移（鼠标垂直移动）
    camTarget = Vector3Add(camTarget, Vector3Scale(camUpDir, mouseDelta.y * panSpeed * camOrbitalDistance * scaleFactor));
}

// ============================================================
// 滚轮缩放：修改正交视口高度（cam.fovy）
// 同时更新轨道距离，保持 zoom 与 pan 速度一致
// ============================================================
void SimRender::zoom(float wheel){
    cam.fovy -= wheel * 2.0f;
    cam.fovy = Clamp(cam.fovy, 2.0f, 200.0f);
    camOrbitalDistance = cam.fovy;
}

// ============================================================
// 将模型参数同步写入 raylib 相机结构体（每帧调用）
// 先重算球面位置，再写入 cam.target 和 cam.position
// ============================================================
void SimRender::camUpdate(){
    recomputeCameraSpherical();
    cam.target = camTarget;
    cam.position = camPosition;
}

// ============================================================
// 获取实体底部中心坐标（double→float转换）
// ============================================================
Vector3 SimRender::getFloatCenter(Entity* e){
    return {(float)e->getX(), (float)e->getY(), (float)e->getZ()};
}

// ============================================================
// 获取实体速度向量（double→float转换）
// ============================================================
Vector3 SimRender::getFloatVel(Entity* e){
    return {(float)e->getVX(), (float)e->getVY(), (float)e->getVZ()};
}

// ============================================================
// 渲染单个实体
// TNT：红色立方体 + 黑色线框，大小 0.98×0.98×0.98
// 珍珠：深绿色小方块 + 黑色线框，大小 0.25×0.25×0.25
// 注意：center.y 上移半个高度，因为实体坐标在底部
// ============================================================
void SimRender::renderEntity(Entity* e){
    Vector3 center = getFloatCenter(e); // 实体底部的中心点

    if (strcmp(e->getName(), "tnt") == 0)
    {
        Color col = RED;
        center.y +=0.98f/2;  // 上移半个TNT高度，使方块居中
        DrawCube(center, 0.98f, 0.98f, 0.98f, col);
        DrawCubeWires(center, 0.98f, 0.98f, 0.98f, BLACK);
    }
    else
    {
        Color col = {24, 62, 12, 200};  // 深绿色（末影珍珠颜色）
        center.y +=0.25f/2;  // 上移半个珍珠高度
        DrawCube(center, 0.25f, 0.25f, 0.25f, col);
        DrawCubeWires(center, 0.25f, 0.25f, 0.25f, BLACK);
    }
}

// ============================================================
// 渲染实体速度箭头
// 黄色圆柱体（箭杆）+ 黄色圆锥体（箭头）
// 速度向量按 velScale 缩放后绘制
// 速度太小时（<0.01）不绘制
// ============================================================
void SimRender::renderVel(Entity* e){
    Vector3 center = getFloatCenter(e); // 实体中心点
    Vector3 vel = getFloatVel(e);
    const float velScale = 2.5f;      // 速度可视化缩放系数
    float len = Vector3Length(vel);
    if(len > 0.01f)
    {
        const float arrowHeadLen  = 0.30f;   // 圆锥箭头固定长度
        const float arrowHeadR    = 0.15f;   // 圆锥底部半径
        const float shaftRadius   = 0.05f;   // 箭杆圆柱半径
        const int   sides         = 16;       // 圆柱/圆锥面数

        // 缩放后的速度向量
        Vector3 dirScaled = Vector3Scale(vel, velScale);
        // 速度方向单位向量
        Vector3 unitDir = Vector3Normalize(dirScaled);

        // 箭杆终点 = 速度向量终点 - 箭头长度
        Vector3 shaftEnd  = Vector3Add(center, Vector3Subtract(dirScaled, Vector3Scale(unitDir, arrowHeadLen)));
        // 箭头尖端 = 速度向量终点
        Vector3 arrowTip = Vector3Add(center, dirScaled);

        // 绘制圆柱箭杆（从中心到 shaftEnd）
        DrawCylinderEx(center, shaftEnd, shaftRadius, shaftRadius, sides, YELLOW);
        // 绘制圆锥箭头（从 shaftEnd 到 arrowTip，半径从头到尾递减）
        DrawCylinderEx(shaftEnd, arrowTip, arrowHeadR, 0.0f, sides, YELLOW);
    }
}

// ============================================================
// 渲染一整帧
// 流程：清屏 → 进入3D模式 → 绘制网格 → 渲染所有实体+速度箭头 → 退出3D模式
// ============================================================
void SimRender::renderFrame(){

    BeginDrawing();
    ClearBackground({64,64,64,255});  // 深灰色背景
    BeginMode3D(cam);
        DrawGrid(30, 1.0f);  // 30×30 参考网格，每格1米

        for (Entity* e : *world->getEntityListPtr())
        {
            renderEntity(e);  // 渲染实体方块
            renderVel(e);     // 渲染速度箭头
        }
    EndMode3D();
}

// ============================================================
// 渲染鼠标悬浮提示框
// 显示：实体名称、tick数、Y坐标、Y速度
// 半透明黑色背景 + 白色文字，跟随鼠标位置
// ============================================================
void SimRender::renderTooltip(Entity* e){
    Vector2 mousePos = GetMousePosition();
    char tooltipBuf[256];
    sprintf(tooltipBuf,
        "%s | tick:%d\nY=%.2f  VY=%.2f",
        e->getName(),
        e->getTick(),
        e->getY(),
        e->getVY()
    );
    // 半透明黑色背景
    DrawRectangle((int)mousePos.x + 14, (int)mousePos.y + 14, 210, 62, Fade(BLACK,0.65f));
    // 白色文字
    DrawText(tooltipBuf, (int)mousePos.x + 18, (int)mousePos.y + 18,14, WHITE);
}

// ============================================================
// 射线拾取：获取鼠标悬浮的最近实体
// 从鼠标位置发射射线，检测与各实体包围盒的碰撞
// 返回距离最近的实体指针，若没有命中则返回 nullptr
// ============================================================
Entity* SimRender::getHoverEntity(){
    Entity* hoverEntity = nullptr;
    Ray mouseRay = GetMouseRay(GetMousePosition(), cam);
    float minDist = 1e10f;  // 初始化为极大值

    for (Entity* e : *world->getEntityListPtr())
    {
        float h = (float)(2.0 * e->getComY());  // 实体高度=2倍眼部高度
        Vector3 center = getFloatCenter(e);
        // 构造包围盒：从底部中心向上延伸
        BoundingBox box;
        box.min = (Vector3){center.x - 0.5f, center.y , center.z - 0.5f};
        box.max = (Vector3){center.x + 0.5f, center.y + h, center.z + 0.5f};

        // 射线与包围盒碰撞检测
        RayCollision col = GetRayCollisionBox(mouseRay, box);
        if (col.hit && col.distance < minDist)
        {
            minDist = col.distance;
            hoverEntity = e;
        }
    }
    return hoverEntity;
}

// ============================================================
// 结束当前帧绘制（调用 raylib 的 EndDrawing）
// ============================================================
void SimRender::endRender(){
    EndDrawing();
}

// ============================================================
// 检查窗口是否关闭（ESC 或 关闭按钮）
// ============================================================
bool SimRender::isDead(){
    return WindowShouldClose();
}

// ============================================================
// 获取当前帧耗时（秒），用于时间节流
// ============================================================
float SimRender::getFrameTime(){
    return GetFrameTime();
}