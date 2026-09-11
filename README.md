# MC Entity Emulator

Minecraft 实体物理仿真工具 —— 通过 **Lua 脚本** 控制 TNT 与末影珍珠的生成、爆炸与回溯，支持 3D 可视化。

---

## 构建

### 依赖

| 依赖 | 用途 | MSYS2 安装 |
|------|------|-----------|
| raylib 5.x | 3D 渲染 / 窗口 | `pacman -S mingw-w64-x86_64-raylib` |
| Lua 5.4 | 脚本引擎 | `pacman -S mingw-w64-x86_64-lua` |
| GCC (C++17) | 编译 | `pacman -S mingw-w64-x86_64-gcc` |

```bash
cd src/build/Debug
g++ -std=c++17 ../../*.cpp \
    -I ../../include \
    $(pkg-config --cflags --libs raylib lua5.4) \
    -lopengl32 -lgdi32 -lwinmm \
    -o outDebug.exe
```

### 运行

```
outDebug.exe <script.lua>
```

### 已知限制

**不支持中文显示。** raylib 的 `DrawText` 使用内置位图字体，仅覆盖 ASCII 字符；Windows 控制台默认编码为 GBK，与源码的 UTF-8 不一致。因此：
- 模拟器窗口内的文本请使用英文
- Lua 脚本的 `mc.publishMessage()` 请使用英文
- 控制台输出请使用英文

---

## 项目结构

```
mc-entity-emulator/
├── include/
│   ├── entities/
│   │   ├── entity.h          # Entity 基类 (坐标/速度/爆炸受击)
│   │   ├── pearl.h           # Pearl : Entity (g=0.03, drag=0.99)
│   │   └── tnt.h             # Tnt   : Entity (g=0.04, drag=0.98)
│   ├── world.h               # World 类 (实体列表/tick/快照)
│   ├── message.h             # Message 结构体 (日志)
│   ├── raylib_tool.h         # SimRender 3D 渲染器
│   └── luaL_regist.h         # registerLuaFunctions() 声明
├── src/
│   ├── main.cpp              # 入口: 初始化→主循环→flow()
│   ├── world.cpp             # World: updateEntityList / 爆炸分发
│   ├── entity.cpp            # Entity 基类实现
│   ├── raylib_tool.cpp       # 3D 渲染器实现
│   └── lua_event_package.cpp # Lua↔C++ 桥接 (所有 mc.* 函数)
├── mcTick flow.txt           # Minecraft tick 流程参考
└── README.md
```

### 全局变量 (定义于 main.cpp)

| 变量 | 类型 | 说明 |
|------|------|------|
| `world` | `World` | 全局唯一世界实例 |
| `L` | `lua_State*` | Lua 虚拟机 (static, main.cpp 内部) |
| `unlimitedSpeedMode` | `bool` | 全速模式开关 |
| `lua_SCRIPT` | `const char*` | 脚本文件路径 (static) |

---

## 仿真 Tick 流程

`flow()` 每 tick 执行:

```
1. luaL_dofile(L, lua_SCRIPT)   -- 重新执行整个脚本，获取当前 tick 判断
2. world.worldNextTick()
   └─ tickGrow()                 -- worldTick += 1
   └─ updateEntityList()
      ├─ 每个实体: nextTick()    -- Entity tick+=1; TNT fuse-=1
      ├─ 每个实体: nextMove()    -- 重力+阻力+位移
      └─ 检查 shouldExpode()
         └─ applyExplosion() 到所有其他实体 → delete TNT
   └─ updateMessageStack()       -- 清理过期消息 (>2 tick)
```

**注意**: `luaL_dofile` 每 tick 重新编译+执行整个脚本。脚本中 `local` 变量在当前 tick 执行完即销毁，需要跨 tick 持久的状态必须用**全局变量**（不加 `local`）。

---

## Lua API

全部函数注册在 `mc` 表下。

### 实体生成

#### `mc.spawnPearl(status, x, y, z, mx, my, mz) → id`

| 参数 | 类型 | 说明 |
|------|------|------|
| status | int | 0=FIXED / 1=FREE |
| x, y, z | number | 初始坐标 (世界坐标) |
| mx, my, mz | number | 初始速度 (米/tick) |
| **返回** | int | 新实体 ID |

物理参数: `g=0.03`, `drag=0.99`, 碰撞箱 `0.25³`

#### `mc.spawnTnt(status, power, fuse, x, y, z, mx, my, mz) → id`

| 参数 | 类型 | 说明 |
|------|------|------|
| status | int | 0=FIXED / 1=FREE |
| power | int | 爆炸威力 (TNT=4) |
| fuse | int | 引信剩余 tick (默认 80 = 4 秒) |
| x, y, z | number | 初始坐标 |
| mx, my, mz | number | 初始速度 |
| **返回** | int | 新实体 ID |

物理参数: `g=0.04`, `drag=0.98`, 碰撞箱 `0.98³`

### 快照与回溯

#### `mc.saveSnapshot(name)`
深拷贝整个世界 (包括所有实体) 保存为快照。同名快照会被覆盖。

#### `mc.loadSnapshot(name)`
用快照覆盖当前世界。**worldTick 也会被恢复**，可能导致 if-tick 块重复触发，需用全局标记位防止。

#### `mc.deleteSnapshot(name)`
释放快照内存。大量快照会占内存，不用时应及时清理。

#### `mc.listSnapshots() → table`
返回 `{name1, name2, ...}` 数组。

### 状态查询

| 函数 | 返回 | 说明 |
|------|------|------|
| `mc.getWorldTick()` | int | 当前 worldTick |
| `mc.getEntityCount()` | int | 当前实体总数 |
| `mc.getEntity(id)` | table 或 nil | 获取实体信息表（见下方字段） |
| `mc.getAllEntityIds()` | table | 所有实体 ID 的数组 `{id1, id2, ...}` |
| `mc.publishMessage(msg)` | nil | 写入世界日志 (UI 左侧面板显示) |

#### `mc.getEntity(id)` 返回的 table 字段

| 字段 | 类型 | 说明 |
|------|------|------|
| `.id` | int | 实体唯一 ID |
| `.name` | string | `"tnt"` 或 `"pearl"` |
| `.tick` | int | 已存活 tick 数 |
| `.status` | int | 0=FIXED / 1=FREE |
| `.x`, `.y`, `.z` | number | 当前坐标 |
| `.mx`, `.my`, `.mz` | number | 当前速度 (m/tick) |
| `.speed` | number | 合速度 = `sqrt(mx²+my²+mz²)` |
| `.alive` | bool | 始终为 `true`（不存在则返回 `nil`） |
| `.fuse` | int | **仅 TNT**：剩余引信 tick |
| `.power` | int | **仅 TNT**：爆炸威力 |

```lua
-- 用法示例
local e = mc.getEntity(someId)
if e and e.speed < 0.01 then
    print("entity " .. e.id .. " stopped at y=" .. e.y)
end

-- 遍历所有实体
for _, id in ipairs(mc.getAllEntityIds()) do
    local e = mc.getEntity(id)
    print(e.name .. ": (" .. e.x .. ", " .. e.y .. ", " .. e.z .. ")")
end
```

---

## C++ API (供扩展参考)

### Entity 基类 (entity.h)

```cpp
class Entity {
protected:
    double x, y, z;           // 坐标
    double mx, my, mz;        // 速度 (m/tick)
    double g;                 // 重力加速度
    double drag;              // 每 tick 乘算阻力
    double bounding_x, y, z;  // 碰撞箱
    double com_x, y, z;       // 爆炸受力点偏移
    int status;               // FIXED=0 / FREE=1
    int tick;                 // 存活 tick 数
    unsigned int id;          // 唯一 ID (World 分配)
    World* my_world;          // 所属世界

public:
    virtual void nextMove() = 0;
    virtual void applyExplosion(double x, double y, double z, int power) = 0;
    virtual bool shouldExpode();           // 默认 false
    virtual int  getExplosionPower();      // 默认 0
    virtual Entity* clone(World*) const = 0; // 深拷贝 (用于快照)
};
```

### World 类 (world.h)

```cpp
class World {
public:
    World();                          // 默认构造
    World(const World& other);        // 深拷贝 (快照用)
    World& operator=(const World& other); // 深拷贝赋值 (回溯用)
    ~World();

    void registerEntity(Entity* e);   // 注册实体 (构造时自动调用)
    void worldNextTick();             // tick+1 + updateEntityList + 过期消息
    void publishMessage(string msg, Color col);

    unsigned int getWorldTick();
    Entity* getEntity(unsigned int id);
    const vector<Entity*>& getEntityListPtr();
};
```

### 爆炸公式

```
distance = sqrt((ex-entity.x)² + (ey-entity.y)² + (ez-entity.z)²)

if distance >= 8: return   (超出半径)

force = max(0, 1 - distance/8) × power

entity.mx += force × (entity.x - ex) / distance
entity.my += force × (entity.y - ey) / distance
entity.mz += force × (entity.z - ez) / distance
```

---

## 操作说明

| 按键 | 功能 |
|------|------|
| `Space` | 单步 1 tick |
| `Shift` (按住) | 连续仿真 (20 TPS) |
| `CapsLock` | 切换全速模式 (帧内尽可能多 tick) |
| `Ctrl + 左键` 点击实体 | 追踪 / 取消追踪实体 |
| `左键` 点击实体 | 终端输出实体详细信息 |
| `右键` 拖动 | 旋转 3D 视角 |
| `中键` 拖动 | 平移视角 |
| `滚轮` | 缩放 |

---

## 示例 Lua 脚本

```lua
-- test.lua: 珍珠空爆 + 回溯演示
local tick = mc.getWorldTick()

if tick == 0 then
    mc.saveSnapshot("origin")
    mc.spawnPearl(0, 0, 100, 0, 2.0, 0.5, 0)
    mc.spawnTnt(0, 4, 80, 2, 100, 0, 1.5, 0.3, 0)
    mc.publishMessage("珍珠 + 推进TNT 已生成")
end

if tick == 80 then
    mc.spawnTnt(0, 4, 0, 2.2, 100.5, 0, 0, 0, 0)
    mc.publishMessage("空爆TNT 已生成")
end

if tick == 200 and not rewinded then
    mc.loadSnapshot("origin")
    mc.spawnPearl(0, 0, 100, 0, 3.0, 1.0, 0)    -- 调整参数
    mc.spawnTnt(0, 6, 80, 2, 100, 0, 2.0, 0.5, 0)
    rewinded = true   -- 全局标记位，防止无限循环
end

if tick == 400 then
    mc.publishMessage("实体数量: " .. mc.getEntityCount())
end
```

### 脚本编写注意事项

1. **每次 tick 整个脚本从头执行** (`luaL_dofile`)，顶层 `if tick == N` 判断是推荐写法
2. **跨 tick 状态用全局变量** (如 `rewinded = true`)，不要用 `local`
3. **loadSnapshot 会重置 worldTick**，回到的 tick 可能重新触发 if 块，必须加标记位防死循环
4. **tick 从 0 开始**，首个 tick 时 `mc.getWorldTick() == 0`
5. **不支持中文**，`mc.publishMessage()` 和所有 UI 文本请使用英文

---

## 添加新的 Lua 函数

1. 在 `lua_event_package.cpp` 中编写 `static int lua_xxx(lua_State* L)` 函数
2. 在 `PACKAGE_FUNCTIONS[]` 数组中注册 `{"xxx", lua_xxx}`
3. 函数签名: `int(lua_State*)`, 参数通过 `luaL_checkinteger/number/string(L, index)` 读取, 返回值通过 `lua_push*` 压栈, `return` 返回值个数
4. 需要访问的全局变量在文件顶部 `extern` 声明

---

## License

MIT