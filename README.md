# Minecraft PE 0.6.1 Win32 Port - Improved

基于 [JackTulli/Minecraft-PE-0.6.1-Win32-port](https://github.com/JackTulli/Minecraft-PE-0.6.1-Win32-port) 的修改版，添加了一些原版 0.6.1 缺失或未完成的功能。

## 该版本目前修改

### 多语言支持

- 移植 0.8.1 语言系统
- 设置界面左侧新增 **Language** 按钮
- 语言界面自动扫描 `data/lang/*.lang`，显示所有可用语言
- 语言名称从 `.lang` 文件里的 `language.name=` 读取
- 切换语言后立即生效，并保存到 `options.txt` 的 `options.language`
- 下次启动自动加载上次选择的语言
- 已内置 `en_US.lang`（英文）和 `zh_CN.lang`（简体中文）

添加新语言教程：

1. 在 `data/lang/` 下新建 `<code>.lang`，比如 `fr_FR.lang`
2. 文件顶部加：

   ```
   language.name=Français
   language.region=FR
   language.code=fr_FR
   ```

3. 保存为 **UTF-8 无 BOM** 编码
4. 重启游戏，语言界面会自动扫描到新语言

### 钓鱼竿

- 右键抛出浮标，再右键收回
- 浮标入水后随机咬钩，收竿可钓到生鱼
- 可钩住生物和方块，收竿时施加拉力
- 鱼线从鱼竿上端连接到浮标
- 浮标使用 `particles.png` 第三行第二列的图标
- 耐久度 65，消耗规则：
  - 钓到鱼：扣 1 点
  - 钩到方块：扣 2 点
  - 钩到实体：扣 3 点
- 创造模式不消耗耐久

### 下界石英矿石

- 在下界 Y=10~117 之间以矿团形式生成
- 每区块尝试 16 次，每次矿团 1~14 个方块
- 挖掘掉落下界石英

### DirectSound 音频后端

- 用 DirectSound 替换原 OpenAL 音频层
- 目前不再需要 `openal.dll`，减少一个运行时依赖
- 已知问题：音乐播放偶尔卡顿、循环异常（见「当前已知 Bug」）

### 设置界面滚动

- 选项太多时支持鼠标拖动滚动
- 支持鼠标滚轮滚动
- 滚动范围自动计算，带边界限制
- 使用 `glScissor` 裁剪，内容不会溢出可视区域

### 鼠标绑定修复

- 刚创建世界时自动抓取鼠标，绑定视角
- 关闭界面后自动恢复鼠标绑定

### 触屏支持修复

- 删除启动时的 `usetouchscreen` 硬编码检测
- `useTouchscreen()` 改为读取设置项，功能完整
- 现已支持触屏操作

### 切石机桌面 UI

- 使用 `DesktopCraftingScreen(3)` 代替原触屏界面
- 与工作台/背包合成界面风格统一

### 桌面容器界面耐久条

- 箱子、合成台、熔炉界面为受损物品绘制耐久条
- 使用 `renderGuiItemDecorations`，绘制顺序：物品图标 → 鼠标物品图标 → 耐久条

### PC 风格创造模式物品栏

- `DesktopCreativeScreen` 提供分页签、可滚动、支持鼠标的完整物品选择界面
- 分页签：全部 / 建筑方块 / 工具 / 食物与护甲 / 装饰 / 机械
- 左键取整组，右键取单个
- 底部显示当前快捷栏，点击可清空对应槽位

### 箱子可以挨着放

- 移除了原版"两个箱子不能相邻"的限制

### 红石灯贴图修正

- 使用正确的点亮贴图

### 传送门方块可挖掘

- `PortalTile::mayPick()` 返回 true
- 破坏时间设为 0.1s

### 方块分类补全

- 为 netherrack、portalTile、grass_carried 等补上 `category`
- 创造模式物品栏不再漏方块

### 设置界面新增飞行选项

- `OPTIONS_IS_FLYING`
- 空格上升、Shift 下降

### 删除 Credits 界面

- 从设置界面移除

### 硬编码 UI 字符串迁移到 I18n

- 覆盖：OptionsScreen、PauseScreen、DeathScreen、InBedScreen、StartMenuScreen、UsernameScreen、SelectWorldScreen、SimpleChooseLevelScreen、JoinGameScreen、JoinByIPScreen、ChestScreen、FurnaceScreen、DesktopCraftingScreen、TouchStartMenuScreen、TouchSelectWorldScreen、TouchJoinGameScreen 等
- `.lang` 里没有对应 key 的字符串（如 `"Cheats: On"`、`"World name:"` 等）保留硬编码

## JackTulli 的修改

### Redstone

A functional, mostly vanilla-faithful redstone subsystem built from the
scaffolding upstream left behind (`Tile::isSignalSource`, `Level::getSignal`,
`hasNeighborSignal`, `hasDirectSignal`). Includes:

- **Redstone dust** — 0..15 strength stored in `auxValue`, decays one per hop,
  cuts properly when the source is removed. Grayscale cross sprite at terrain
  `(4, 10)` is tinted red at render time via `getColor` — bright when powered,
  dim when off so you can see the wire layout.
- **Lever** — wall-mountable on any solid face (and floor). `auxValue` bit
  `0x8` is on/off, bits `0..2` are mount face (1..5, same convention as torch).
  No proper lever sprite exists in this build's atlas, so the body renders as
  a small wooden cube that flips to cobblestone when thrown.
- **Button** — wall-mountable like the lever, with a timed press: 20 ticks for
  stone, 30 for wood. `setShape` flattens the AABB against whichever face
  it's mounted on.
- **Pressure plate** — random-ticks itself to scan for entities in a small
  AABB above the plate, sets aux 0/1, fires neighbor updates.
- **Redstone torch** — two tile IDs (`notGate_off` = 75, `notGate_on` = 76)
  sharing a class with a `_lit` flag. Acts as a NOT-gate: schedules a 2-tick
  state-check whenever neighbours change, swaps variant in `tick`. Always
  strongly powers the block directly above (regardless of mount), so
  `torch → dirt → door` opens the door.
- **Redstone lamp** — two-variant tile (123 off, 124 on) that lights when any
  signal touches it. Off form uses a baked "opaque glass" texture (the glass
  speckle pattern composited onto a warm-white backing, written into the
  previously-unused atlas cell at `(4, 11)`); on form uses the glowstone
  sprite with full light emission.
- **Doors / TNT** react to weak power via the existing `hasNeighborSignal`
  path — nothing else needed there. Wires also do an "indirect neighbour
  update" (neighbours-of-neighbours) on every strength change so torches and
  components attached to wire-adjacent blocks notice the change.

### `/time` 命令

`/time set day|noon|night|midnight` 和 `/time set <ticks>` 现在可以正常工作。
本版本的昼夜循环是 19200 tick（Java 版是 24000），所以符号表锚定到这个
周期（`day`/`noon` → `TICKS_PER_DAY/4`，`night`/`midnight` →
`3*TICKS_PER_DAY/4`），数字输入从原版的 0..23999 范围重新缩放，因此
`/time set 6000` 仍然表示中午。

### HUD

生命值和护甲条移动到 Java 版的位置（快捷栏上方）。生命值贴快捷栏左边缘；
护甲条右对齐到快捷栏右边缘，中间留硬间隔，两行不接触。

### 存档 / 读取

`LevelData::setTagData` / `getTagData` 现在序列化当前 `Dimension`。此前从
下界保存退出后重新载入，会在主世界用下界坐标加载——如果那些坐标在水平面
以下或墙里，会立即死亡。现在 Dimension 跨重载保留。

### 桶

- 空桶的射线检测现在忽略"跳过液体"模式，右键水面会真正命中水面而非下方的沙块
- 如果射线命中水旁边的固体方块，桶会回退到面相邻格并拾取那里的液体
- 桶放置时放置的是**动态**流体 id（水 8、岩浆 10），并调度 tick 让它真正开始流动

### 开始界面

从 `StartMenuScreen` 和 `TouchStartMenuScreen` 中移除了 Kolyah35 致谢文本、
GitHub 图标 blit 和可点击 URL 处理器。

### Windows 2000 / XP 兼容性

OpenGL 上下文请求从 2.1 降到 1.1（`main_glfw.h`），这样 Mesa3D 的软件
光栅化器（Win2K 扩展内核 VM 最终会用的）能满足请求。一个 32 位、自包含的
Mesa3D 17.3.7 `opengl32.dll` 签入在 `win2k/` 下；在原生 OpenGL 驱动不达标
的系统上，把它复制到 `MinecraftPE.exe` 旁边即可。更新的 Mesa 分支（20.x、
26.x）在带扩展内核的 Win2K 上无法加载——17.3.7 是 `pal1000` 显式针对旧
Windows ABI 构建的最后一个版本。

## 构建

支持的构建环境：**Windows + Visual Studio 2017 生成工具**，使用
`v141_xp` 工具集。PowerShell 脚本会处理 vcvars 环境、选择对应的
Ninja，并将输出程序生成至 `build-xp\MinecraftPE.exe`。

```powershell
# 在仓库根目录执行
powershell.exe -ExecutionPolicy Bypass -File .\build-xp.ps1

# 清理后重新构建
.\build-xp.ps1 -Clean
```

**依赖要求**：

- 已安装带有 `v141_xp`（Windows XP）工具集的 Visual Studio 2017 生成工具。
  脚本固定使用 `-vcvars_ver=14.16`，避免新版本工具集被静默调用。
- CMake 3.21 或更高版本。
- Ninja 已加入系统 `PATH` 环境变量（也可以修改 `build-xp.ps1` 内的 `$NinjaDir` 参数）。
- Git（部分 CMake `FetchContent` 依赖会在配置阶段拉取克隆）。

编译输出文件为 `build-xp\MinecraftPE.exe`。exe 同级的 `data\` 文件夹存放游戏资源
（terrain.png、items.png、语言文件等）——构建过程会自动把这些资源复制到 `build-xp\data\`。

### 在 Windows 2000（扩展内核）上运行

1. 按上方步骤完成构建。
2. 将 `build-xp\MinecraftPE.exe` 以及同级的 `data\` 文件夹复制到目标机器。
3. 把 `win2k\opengl32.dll`（约 24MB）复制到 MinecraftPE.exe 所在目录。
   **不要**覆盖 `C:\WINNT\system32\opengl32.dll`——将 Mesa 版本放在程序目录，
   仅对本游戏生效。
4. 可选：启动前设置环境变量 `set GALLIUM_DRIVER=softpipe`，强制使用纯 C
   软件渲染器。默认 llvmpipe 为 JIT 渲染，需要 SSE2 指令集，遇到兼容性
   问题时使用该选项。

## 控制按键

| 按键 | 功能 |
| --- | --- |
| W A S D | 移动 |
| 鼠标 | 视角转动 |
| 鼠标左键 | 破坏方块 |
| 鼠标右键 | 放置方块 |
| 空格 | 跳跃 |
| Shift | 潜行 |
| E | 打开背包 |
| ESC | 暂停 / 退出 |
| F1 | 隐藏 HUD 界面 |
| F3 | 调试信息（帧率、坐标） |

> 鼠标捕获：运行游戏后鼠标会被窗口捕获；按 Alt+Tab 切出窗口释放鼠标。

## 当前已知 Bug

1. 音乐播放偶尔卡顿、循环异常（DirectSound 音频层移植缺陷）
2. 部分粒子效果渲染错乱（火焰、爆炸粒子）
3. 水面渲染有瑕疵，透明方块有 Z-fighting（贴图闪烁重叠）
4. 存档兼容：只能读取 PE 0.6.1 原版存档；更高版本存档不能载入
5. 偶尔退出游戏发生内存泄漏，程序崩溃
6. XP 系统下，音频初始化失败概率较高

## 开发笔记

| 问题 | 解决方案 |
| --- | --- |
| 改 `.h` 没 `-Clean` → `0xc0000005` | 改 `.h` 后必须 `-Clean` |
| `-Clean` 时 CMake 下载 zlib 失败 | 开代理 / VPN |
| `OreFeature::place()` 走 Level 递归崩溃 | 直接操作 `blocks[]` 数组 |
| `git push` 报 rejected | 先 `git pull origin main` 再 push |
| Option 自动加 `options.` 前缀 | key 写 `language`（不带前缀） |
| `renderGuiItemDecorations` 耐久条被覆盖 | 绘制顺序：图标 → 鼠标图标 → 耐久条 |
| `Tesselator` 未定义 | `#include "../../../renderer/Tesselator.h"` |
| `const char* x = I18n::get(...).c_str()` 悬空 | 改成 `std::string x = I18n::get(...)` |
| 静态 `I18n::get()` 在 `main()` 前执行导致崩溃 | 改为函数内懒初始化或硬编码 |

## 致谢

- 原项目：[JackTulli/Minecraft-PE-0.6.1-Win32-port](https://github.com/JackTulli/Minecraft-PE-0.6.1-Win32-port)
- 钓鱼竿参考：4J Studios 主机版实现
- 多语言系统参考：[电灯泡LamPbulB](https://github.com/coj211/MCCE)

## 许可

本项目仅供学习和研究使用。Minecraft 为 Mojang AB 的商标，
本项目与 Mojang 无关联。
