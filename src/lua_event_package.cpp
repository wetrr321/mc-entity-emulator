// ============================================================
// lua_event_package.cpp - Lua 可调用的 C++ 函数注册
// 提供 spawnTnt / spawnPearl / 快照回溯等功能
// 用户在 Lua 脚本中直接调用这些函数来控制仿真
// ============================================================

#include "world.h"
#include "entities/tnt.h"
#include "entities/pearl.h"

#include <unordered_map>
#include <string>
#include <cmath>

#include "raylib.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

// ========== 外部全局变量（定义在 main.cpp） ==========
extern World world;
extern bool unlimitedSpeedMode;

// ========== 快照存储 ==========
static std::unordered_map<std::string, World> snapshots;

// ============================================================
// spawnPearl(status, x, y, z, mx, my, mz)
// 生成一个末影珍珠实体
// 返回：新实体的 id
// ============================================================
static int lua_spawnPearl(lua_State* L)
{
    int nargs = lua_gettop(L);
    if (nargs < 7) {
        lua_pushstring(L, "spawnPearl needs 7 args: status, x, y, z, mx, my, mz");
        lua_error(L);
        return 0;
    }

    int    status = (int)luaL_checkinteger(L, 1);
    double x      = luaL_checknumber(L, 2);
    double y      = luaL_checknumber(L, 3);
    double z      = luaL_checknumber(L, 4);
    double mx     = luaL_checknumber(L, 5);
    double my     = luaL_checknumber(L, 6);
    double mz     = luaL_checknumber(L, 7);

    Pearl* pearl = new Pearl(&world, status, x, y, z, mx, my, mz);

    lua_pushinteger(L, pearl->getId());
    return 1;
}

// ============================================================
// spawnTnt(status, power, fuse, x, y, z, mx, my, mz)
// 生成一个 TNT 实体
// 返回：新实体的 id
// ============================================================
static int lua_spawnTnt(lua_State* L)
{
    int nargs = lua_gettop(L);
    if (nargs < 9) {
        lua_pushstring(L, "spawnTnt needs 9 args: status, power, fuse, x, y, z, mx, my, mz");
        lua_error(L);
        return 0;
    }

    int    status = (int)luaL_checkinteger(L, 1);
    int    power  = (int)luaL_checkinteger(L, 2);
    int    fuse   = (int)luaL_checkinteger(L, 3);
    double x      = luaL_checknumber(L, 4);
    double y      = luaL_checknumber(L, 5);
    double z      = luaL_checknumber(L, 6);
    double mx     = luaL_checknumber(L, 7);
    double my     = luaL_checknumber(L, 8);
    double mz     = luaL_checknumber(L, 9);

    Tnt* tnt = new Tnt(&world, status, power, fuse, x, y, z, mx, my, mz);

    lua_pushinteger(L, tnt->getId());
    return 1;
}

// ============================================================
// saveSnapshot("name")
// 保存当前世界状态到快照（深拷贝）
// ============================================================
static int lua_saveSnapshot(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    if (name == nullptr || name[0] == '\0') {
        lua_pushstring(L, "saveSnapshot: name cannot be empty");
        lua_error(L);
        return 0;
    }

    snapshots[name] = world;
    world.publishMessage("Snapshot saved: " + std::string(name), GREEN);
    return 0;
}

// ============================================================
// loadSnapshot("name")
// 从快照恢复世界状态
// ============================================================
static int lua_loadSnapshot(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);

    auto it = snapshots.find(name);
    if (it == snapshots.end()) {
        lua_pushfstring(L, "loadSnapshot: snapshot '%s' not found", name);
        lua_error(L);
        return 0;
    }

    world = it->second;
    world.publishMessage("Snapshot loaded: " + std::string(name), YELLOW);
    return 0;
}

// ============================================================
// deleteSnapshot("name")
// 删除指定快照，释放内存
// ============================================================
static int lua_deleteSnapshot(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    snapshots.erase(name);
    return 0;
}

// ============================================================
// listSnapshots()
// 返回所有快照名称的列表（Lua table）
// ============================================================
static int lua_listSnapshots(lua_State* L)
{
    lua_newtable(L);
    int i = 1;
    for (auto& kv : snapshots) {
        lua_pushstring(L, kv.first.c_str());
        lua_rawseti(L, -2, i++);
    }
    return 1;
}

// ============================================================
// getWorldTick()
// 返回当前世界 tick 数
// ============================================================
static int lua_getWorldTick(lua_State* L)
{
    lua_pushinteger(L, world.getWorldTick());
    return 1;
}

// ============================================================
// getEntityCount()
// 返回当前世界中的实体数量
// ============================================================
static int lua_getEntityCount(lua_State* L)
{
    lua_pushinteger(L, (lua_Integer)world.getEntityListPtr().size());
    return 1;
}

// ============================================================
// publishMessage("msg")
// 向世界日志发布一条消息
// ============================================================
static int lua_publishMessage(lua_State* L)
{
    const char* msg = luaL_checkstring(L, 1);
    world.publishMessage(msg, WHITE);
    return 0;
}

// ============================================================
// getEntity(id)
// 返回实体信息表，可通过 e.x e.y 访问
// 若实体不存在则返回 nil
// ============================================================
static int lua_getEntity(lua_State* L)
{
    unsigned int id = (unsigned int)luaL_checkinteger(L, 1);
    Entity* e = world.getEntity(id);
    if (e == nullptr) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);

    lua_pushstring(L, "id");     lua_pushinteger(L, e->getId());        lua_settable(L, -3);
    lua_pushstring(L, "name");   lua_pushstring(L, e->getName());       lua_settable(L, -3);
    lua_pushstring(L, "tick");   lua_pushinteger(L, e->getTick());      lua_settable(L, -3);
    lua_pushstring(L, "status"); lua_pushinteger(L, e->getStatus());    lua_settable(L, -3);

    lua_pushstring(L, "x");      lua_pushnumber(L, e->getX());          lua_settable(L, -3);
    lua_pushstring(L, "y");      lua_pushnumber(L, e->getY());          lua_settable(L, -3);
    lua_pushstring(L, "z");      lua_pushnumber(L, e->getZ());          lua_settable(L, -3);

    lua_pushstring(L, "mx");     lua_pushnumber(L, e->getVX());         lua_settable(L, -3);
    lua_pushstring(L, "my");     lua_pushnumber(L, e->getVY());         lua_settable(L, -3);
    lua_pushstring(L, "mz");     lua_pushnumber(L, e->getVZ());         lua_settable(L, -3);

    double vx = e->getVX(), vy = e->getVY(), vz = e->getVZ();
    double speed = std::sqrt(vx*vx + vy*vy + vz*vz);
    lua_pushstring(L, "speed");  lua_pushnumber(L, speed);              lua_settable(L, -3);

    lua_pushstring(L, "alive");  lua_pushboolean(L, 1);                 lua_settable(L, -3);

    if (e->getExplosionPower() > 0) {
        lua_pushstring(L, "fuse");  lua_pushinteger(L, ((Tnt*)e)->getFuse());   lua_settable(L, -3);
        lua_pushstring(L, "power"); lua_pushinteger(L, e->getExplosionPower()); lua_settable(L, -3);
    }

    return 1;
}

// ============================================================
// getAllEntityIds()
// 返回当前世界所有实体 ID 的数组
// 与 getEntity(id) 配合遍历
// ============================================================
static int lua_getAllEntityIds(lua_State* L)
{
    lua_newtable(L);
    int i = 1;
    for (Entity* e : world.getEntityListPtr()) {
        lua_pushinteger(L, e->getId());
        lua_rawseti(L, -2, i++);
    }
    return 1;
}
static const luaL_Reg PACKAGE_FUNCTIONS[] = {
    {"spawnPearl",      lua_spawnPearl},
    {"spawnTnt",        lua_spawnTnt},
    {"saveSnapshot",    lua_saveSnapshot},
    {"loadSnapshot",    lua_loadSnapshot},
    {"deleteSnapshot",  lua_deleteSnapshot},
    {"listSnapshots",   lua_listSnapshots},
    {"getWorldTick",    lua_getWorldTick},
    {"getEntityCount",  lua_getEntityCount},
    {"publishMessage",  lua_publishMessage},
    {"getEntity",       lua_getEntity},
    {"getAllEntityIds", lua_getAllEntityIds},
    {NULL, NULL}
};

// ============================================================
// registerLuaFunctions(L)
// 将所有 C 函数注册为 Lua 全局函数
// 应在 luaL_newstate() 之后、加载脚本之前调用一次
// ============================================================
void registerLuaFunctions(lua_State* L)
{
    luaL_newlib(L, PACKAGE_FUNCTIONS);
    lua_setglobal(L, "mc");
}