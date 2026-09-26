# Minecraft PE 0.6.1 Win32 Port - Improved

基于 [JackTulli/Minecraft-PE-0.6.1-Win32-port](https://github.com/JackTulli/Minecraft-PE-0.6.1-Win32-port) 的修改版，添加了一些原版 0.6.1 缺失或未完成的功能

## 该版本目前修改

### 多语言支持

- 移植0.8.1语言系统
- 设置界面左侧新增 **Language** 按钮
- 语言界面自动扫描 `data/lang/*.lang`，显示所有可用语言
- 语言名称从 `.lang` 文件里的 `language.name=` 读取
- 切换语言后立即生效，并保存到 `options.txt` 的 `options.language`
- 下次启动自动加载上次选择的语言
- 已内置 `en_US.lang`（英文）和 `zh_CN.lang`（简体中文）
- 添加新语言教程
1. 在 `data/lang/` 下新建 `<code>.lang`，比如 `fr_FR.lang`
2. 文件顶部加：

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

### directsound替换，目前不再需要openal.dll

### 设置界面滚动

- 选项太多时支持鼠标拖动滚动
- 支持鼠标滚轮滚动
- 滚动范围自动计算，带边界限制
- 使用 `glScissor` 裁剪，内容不会溢出可视区域

### 鼠标绑定修复

- 刚创建世界时自动抓取鼠标，绑定视角
- 关闭界面后自动恢复鼠标绑定

### 删除启动时usetouchscreen检测，修复usetouchscreen全部功能，现已支持触屏

## JackTulli的修改
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

### `/time` command
`/time set day|noon|night|midnight` and `/time set <ticks>` now work
correctly. This build's day cycle is 19200 ticks (vs Java's 24000), so the
symbol table is anchored to this cycle (`day`/`noon` → `TICKS_PER_DAY/4`,
`night`/`midnight` → `3*TICKS_PER_DAY/4`), and numeric input is rescaled from
vanilla's 0..23999 range so `/time set 6000` still means noon.

### HUD
Hearts and armor moved to their Java positions on top of the hotbar. Hearts
sit at the left edge of the hotbar; armor bar is right-anchored to the right
edge of the hotbar with a hard gap so the two rows don't touch.

### Save / load
`LevelData::setTagData` / `getTagData` now serialize the current `Dimension`.
Previously, saving and quitting from the Nether reloaded the world in the
overworld at the Nether coordinates — instant death if those were below
sea level or inside a wall. Dimension is now preserved across reloads.

### Buckets
- Empty bucket raycast now ignores liquid-skipping mode, so right-clicking
  water actually hits the water surface instead of the sand block underneath.
- If the raycast lands on a solid block adjacent to water, the bucket falls
  back to the face-neighbour cell and picks up the liquid there.
- Bucket placement places the *dynamic* fluid id (water 8, lava 10) and
  schedules a tick so it actually starts flowing.

### Start screens
Removed the Kolyah35 credit text, GitHub icon blit, and clickable URL handler
from both `StartMenuScreen` and `TouchStartMenuScreen`.

### Windows 2000 / XP compatibility
The OpenGL context request was lowered from 2.1 to 1.1 in `main_glfw.h` so
Mesa3D's software rasteriser (which is what Win2K's extended-kernel VMs end
up using) can fulfill it. A 32-bit, self-contained Mesa3D 17.3.7
`opengl32.dll` is checked in under `win2k/`; copy it next to `MinecraftPE.exe`
on systems whose native OpenGL driver doesn't make the cut. Newer Mesa
branches (20.x, 26.x) won't load on Win2K with the extended kernel — 17.3.7
is the last release `pal1000` built explicitly against the older Windows
ABI.

## 支持系统

- Windows XP（32 位 / 64 位）
- Windows 7
- Windows 8 / 8.1
- Windows 10
- Windows 11

> 
> 32 位与 64 位系统均可运行；XP 需要用专门的构建脚本。

## 依赖项编译要求

### Windows（普通现代系统）

1. CMake 3.18 或更新
2. Visual Studio 2019 / 2022（带 C++ 桌面开发组件）
3. Git
4. Python3（用于资源处理脚本）

### Windows XP 编译

XP 不能用新版 VS，仓库提供`build‑xp.ps1`，使用旧版工具链。

> 
> 注意：XP 编译目标只能生成 XP 兼容二进制，在新系统也能运行。

### 外部库

- glad（OpenGL 加载器，仓库已内置）
- zlib（压缩库）
- libpng（读取 PNG 纹理）
- OpenAL‑Soft（目前已不再需要）

## 编译步骤

### 1. 克隆仓库

```
git clone https://github.com/guifeilun/Minecraft0.6.1-improvement-win32-pc-port.git
cd Minecraft0.6.1-improvement-win32-pc-port
git submodule update --init --recursive
```

### 编译

```powershell
.\build-xp.ps1 -Clean

### 3控制按键

表格

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

> 
> 鼠标捕获：运行游戏后鼠标会被窗口捕获；按 Alt+Tab 切出窗口释放鼠标。

## 当前已知 Bug

1. 音乐播放偶尔卡顿、循环异常（OpenAL 音频层移植缺陷）
2. 部分粒子效果渲染错乱（火焰、爆炸粒子）
3. 水面渲染有瑕疵，透明方块有 Z‑fighting（贴图闪烁重叠）
4. 存档兼容：只能读取 PE0.6.1 原版存档；更高版本存档不能载入。
5. 偶尔退出游戏发生内存泄漏，程序崩溃。
6. XP 系统下，音频初始化失败概率较高。
