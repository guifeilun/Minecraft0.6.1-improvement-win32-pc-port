# Minecraft PE 0.6.1 Win32 移植版 - 改进版

基于 [JackTulli/Minecraft-PE-0.6.1-Win32-port](https://github.com/JackTulli/Minecraft-PE-0.6.1-Win32-port) 的修改版，添加了一些原版 0.6.1 缺失或未完成的功能。

## 该版本目前修改

### 无限世界

- 创建世界界面新增**世界类型**按钮：`世界类型：普通` / `世界类型：无限`
- **有限世界**（旧 256×256）和**无限世界**（程序生成，无边界）可自由切换
- 无限世界区块按 32×32 分 region 存储，文件命名 `chunks.r.<rx>.<rz>.dat`
- 有限世界保持原有 `chunks.dat` 单文件存储，旧存档完全兼容
- 出生点：无限世界从 (8, 8) 开始搜索，有限世界保持原逻辑
- 走远、存档、重进，位置和地形都保留

### 多语言支持

- 移植 0.8.1 语言系统
- 设置界面左侧新增 **Language** 按钮
- 语言界面自动扫描 `data/lang/*.lang`，显示所有可用语言
- 语言名称从 `.lang` 文件里的 `language.name=` 读取
- 切换语言后立即生效，并保存到 `options.txt` 的 `options.language`
- 下次启动自动加载上次选择的语言
- 已内置 `en_US.lang`（英文）和 `zh_CN.lang`（简体中文）
- 硬编码 UI 字符串迁移到 I18n，覆盖几乎全部菜单

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

### 合成表

- 更新合成表，现在各种物品均可合成

### DirectSound 音频后端

- 用 DirectSound 替换原 OpenAL 音频层
- 目前不再需要 `openal.dll`，减少一个运行时依赖
- 已知问题：音乐播放偶尔卡顿、循环异常（见「当前已知 Bug」）

### 设置界面滚动

- 选项太多时支持鼠标拖动滚动
- 支持鼠标滚轮滚动
- 滚动范围自动计算，带边界限制
- 使用 `glScissor` 裁剪，内容不会溢出可视区域
- 加入 `isFlying` 选项

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

### 切维度稳定性修复

- 修复下界 ↔ 主世界切换后崩溃（实体未从 chunk 移除导致 `saveAll` 访问悬空指针）
- 加固 `ExplodePacket` 网络包解析（防止损坏包导致越界）
- 修复 `MeleeAttackGoal` 和 `PathNavigation` 共享 `Path*` 的双重删除

## JackTulli 的修改

### 红石

一套功能可用、基本忠于原版的红石系统，构建于上游遗留的脚手架
（`Tile::isSignalSource`、`Level::getSignal`、`hasNeighborSignal`、
`hasDirectSignal`）之上。包括：

- **红石粉** —— 强度 0..15 存储在 `auxValue`，每跳衰减 1，电源移除后正确截断。
  地形图集 `(4, 10)` 处的灰度十字贴图在渲染时通过 `getColor` 染红 —— 有电时亮，
  无电时暗，方便查看线路布局。
- **拉杆** —— 可安装在任意实心面（包括地面）上。`auxValue` 位 `0x8` 是开/关，
  位 `0..2` 是安装面（1..5，与火把相同的约定）。该版本的图集中没有合适的拉杆贴图，
  所以主体渲染为一个可翻转的小木块，切换时变成圆石。
- **按钮** —— 与拉杆一样可安装在墙上，带定时按压：石头 20 tick，木头 30 tick。
  `setShape` 会将 AABB 压扁到它安装的那个面上。
- **压力板** —— 随机 tick 扫描板上方小 AABB 内的实体，设置 aux 0/1，触发邻居更新。
- **红石火把** —— 两个 tile ID（`notGate_off` = 75，`notGate_on` = 76），
  共用一个类，带 `_lit` 标志。作为非门：邻居变化时安排 2 tick 状态检查，
  在 `tick` 中交换变体。始终强充能正上方的方块（无论安装面），所以
  `火把 → 泥土 → 门` 能打开门。
- **红石灯** —— 双变体 tile（123 关，124 开），任何信号触碰时点亮。
  关的形式使用烘焙的"不透明玻璃"贴图（玻璃斑点图案叠加在暖白色背景上，
  写入之前未使用的图集单元 `(4, 11)`）；开的形式使用荧石贴图，全亮度发光。
- **门 / TNT** 通过现有的 `hasNeighborSignal` 路径响应弱电 —— 无需额外处理。
  电线每次强度变化时还会做一次"间接邻居更新"（邻居的邻居），
  以便连接到电线相邻方块的拉杆和组件能感知变化。

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
| 切维度后崩在 `GoalSelector::_Umove` | `switchDimension` 的 `delete e` 前没从 chunk 移除，`saveAll` 遍历 chunk 时访问悬空 `e`。修法：`delete e` 前 `if (e->inChunk) lc->removeEntity(e, e->yChunk);` |
| `MeleeAttackGoal` 和 `PathNavigation` 共享 `Path*` | 两个都持有 `Path*`，一个删了另一个悬空。修法：`MeleeAttackGoal` 不删 `path`，交给 `PathNavigation` |

## 致谢

- 原项目：[JackTulli/Minecraft-PE-0.6.1-Win32-port](https://github.com/JackTulli/Minecraft-PE-0.6.1-Win32-port)
- 钓鱼竿参考：4J Studios 主机版实现
- 多语言系统参考：[电灯泡LamPbulB](https://github.com/coj211/MCCE)

## 许可

本项目仅供学习和研究使用。Minecraft 为 Mojang AB 的商标，
本项目与 Mojang 无关联。
