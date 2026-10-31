# Minecraft PE 0.6.1 Win32 Port - Improved

<!-- README-I18N:START -->
**English** | [简体中文](./README.zh-CN.md)
<!-- README-I18N:END -->

A modified version based on [JackTulli/Minecraft-PE-0.6.1-Win32-port](https://github.com/JackTulli/Minecraft-PE-0.6.1-Win32-port), adding some features missing or unfinished in the original 0.6.1. Multi-platform builds are also completed (Windows / Linux / macOS / Android / Web).


3. Save as **UTF-8 without BOM**
4. Restart the game; the language screen will automatically detect the new language

### New Desktop Container UI

- Chests, crafting tables, furnaces, stonecutters, and the creative inventory all use the **desktop container interface**
- Uses the PE native `gui/spritesheet.png` nine-patch (panel background, slots, borders, selection highlight, tabs)
- A unified close button is placed in the top-right corner
- The crafting interface adds a 2x2 survival inventory layout (armor slots + 2x2 crafting grid + result slot + player model area)
- Uses the native arrow texture between the result slot and the crafting grid
- The stonecutter uses `DesktopCraftingScreen(3)` instead of the original touch interface
- All interfaces draw durability bars for damaged items (draw order: icon → cursor icon → durability bar)
- Double-click items to batch collect/distribute (cursor model)

### Boat

- **Complete entity**: Implements an independent boat entity, supporting placement and movement on water and land.
- **Placement logic**: Uses raycast placement; correctly spawns on water and shore.
- **Driving experience**:
- Press `W` to move forward, `A` / `D` to steer.
- The boat's heading smoothly rotates with the movement direction.
- No reverse (`S` key does nothing), matching the feel of early versions.
- **Riding interaction**:
- Right-click the boat to board.
- Press `Shift` (or the Android sneak key) to dismount; the player is placed above the boat.
- The player adopts a bent-leg sitting pose while on the boat.
- **Physics and visuals**:
- Includes buoyancy calculation, speed limits, and water wave bobbing.
- The boat body has a left-right rocking feedback on collision (when hit).
- The boat model uses hand-drawn elongated blocks (with a recessed cabin), reusing the wood plank texture from `terrain.png`.
- **Survival attributes**:
- The boat has 4 health points and regenerates 1 point per second.
- Takes damage when attacked; drops a boat item when health reaches zero.
- Supports save persistence; boat position and health are correctly saved and restored.

### Spawn Eggs

- **Complete item**: Replicates the 0.8.1 spawn egg mechanism, using the native gray-with-green-spots egg texture.
- **Multi-mob support**: Supports spawning 9 early-version mobs (chicken, cow, pig, sheep, zombie, creeper, skeleton, spider, zombie pigman).
- **Attribute mapping**:
- Uses a single ID + `auxValue` (0~8) to distinguish different mobs.
- Different spawn eggs cannot stack, making sorting and picking easier.
- **Right-click spawning**:
- Right-click on a block to spawn the corresponding entity on the adjacent block above.
- Consumes the item in survival mode; does not consume in creative mode.
- **Creative mode adaptation**:
- Special handling in the creative inventory automatically expands and displays 9 different spawn eggs in an orderly layout.
- **Name reuse**:
- No need to duplicate localization text; spawn egg names automatically reuse the underlying entity names (e.g., "Pig", "Zombie").
- **Underlying calls**:
- Directly uses the native `MobFactory::CreateMob` to create entities; spawned mobs have complete native AI and behavior logic.

### Fishing Rod

- Right-click to cast the bobber; right-click again to reel in
- After the bobber enters water, it randomly bites; reeling in can catch raw fish
- Can hook mobs and blocks; applies pulling force when reeling in
- The fishing line connects from the top of the rod to the bobber
- The bobber uses the icon at row 3, column 2 of `particles.png`
- Durability 65, consumption rules:
- Catch a fish: -1
- Hook a block: -2
- Hook an entity: -3
- No durability consumption in creative mode

### Nether Quartz Ore

- Generates in the Nether between Y=10~117 as ore clusters
- 16 attempts per chunk, each cluster 1~14 blocks
- Mining drops Nether Quartz

### Crafting Recipes

- Updated crafting recipes; various items can now be crafted

### DirectSound Audio Backend

- Replaced the original OpenAL audio layer with DirectSound
- No longer requires `openal.dll`, reducing one runtime dependency
- Known issues: occasional music playback stuttering and looping anomalies (see "Current Known Bugs")

### Settings Screen Scrolling

- Supports mouse drag scrolling when there are too many options
- Supports mouse wheel scrolling
- Scroll range is automatically calculated with boundary limits
- Uses `glScissor` clipping so content does not overflow the visible area
- Added the `isflying` option

### Mouse Binding Fix

- Automatically captures the mouse and binds the view when a world is first created
- Automatically restores mouse binding after closing the UI

### Touchscreen Support Fix

- Removed the `usetouchscreen` hardcoded detection at startup
- `useTouchscreen()` now reads the settings option; functionality is complete
- Touchscreen operation is now supported

### Chests Can Be Placed Adjacent

- Removed the original "two chests cannot be adjacent" restriction

### Redstone Lamp Texture Fix

- Uses the correct lit texture

### Portal Blocks Are Mineable

- `PortalTile::mayPick()` returns true
- Break time set to 0.1s

### Block Category Completion

- Added `category` for netherrack, portalTile, grass_carried, etc.
- Creative inventory no longer misses blocks

### Added Flight Option in Settings

- `OPTIONS_IS_FLYING`
- Space to ascend, Shift to descend

### Removed Credits Screen

- Removed from the settings screen

## JackTulli's Modifications

### Redstone

A functional, mostly vanilla-faithful redstone system built on the upstream leftover scaffolding (`Tile::isSignalSource`, `Level::getSignal`, `hasNeighborSignal`, `hasDirectSignal`). Includes:

- **Redstone Dust** — strength 0..15 stored in `auxValue`, decays by 1 per hop, correctly truncates when power is removed. The grayscale cross texture at atlas `(4, 10)` is tinted red at render time via `getColor` — bright when powered, dark when unpowered, making circuit layouts easy to see.
- **Lever** — can be mounted on any solid face (including the ground). `auxValue` bit `0x8` is on/off, bits `0..2` are the mounting face (1..5, same convention as torches). This version's atlas has no suitable lever texture, so the main body renders as a small flip-able wooden block that turns into cobblestone when toggled.
- **Button** — like the lever, can be mounted on walls, with timed press: stone 20 ticks, wood 30 ticks. `setShape` flattens the AABB onto the face it is mounted on.
- **Pressure Plate** — randomly ticks to scan entities within a small AABB above the plate, sets aux 0/1, and triggers neighbor updates.
- **Redstone Torch** — two tile IDs (`notGate_off` = 75, `notGate_on` = 76), sharing one class with a `_lit` flag. Acts as an inverter: schedules a 2-tick state check on neighbor changes, swaps variants in `tick`. Always strongly powers the block directly above it (regardless of mounting face), so `torch → dirt → door` opens the door.
- **Redstone Lamp** — dual-variant tile (123 off, 124 on), lights up when touched by any signal. The off form uses a baked "opaque glass" texture (glass speckle pattern over a warm white background, written into the previously unused atlas cell `(4, 11)`); the on form uses the glowstone texture, glowing at full brightness.
- **Doors / TNT** respond to weak power via the existing `hasNeighborSignal` path — no extra handling needed. Wires also perform an "indirect neighbor update" (neighbor's neighbor) on every strength change so that levers and components connected to adjacent blocks can sense the change.

### `/time` Command

`/time set day|noon|night|midnight` and `/time set <ticks>` now work correctly.
This version's day/night cycle is 19200 ticks (Java is 24000), so the symbol table
is anchored to this cycle (`day`/`noon` → `TICKS_PER_DAY/4`, `night`/`midnight` →
`3*TICKS_PER_DAY/4`), and numeric input is rescaled from the vanilla 0..23999 range,
so `/time set 6000` still means noon.

### HUD

Health and armor bars moved to the Java Edition position (above the hotbar). Health is
flush with the left edge of the hotbar; the armor bar is right-aligned to the hotbar's
right edge, with a hard gap in the middle, the two rows not touching.

### Save / Load

`LevelData::setTagData` / `getTagData` now serializes the current `Dimension`. Previously,
after saving and exiting from the Nether and reloading, the Overworld would load with
Nether coordinates — if those coordinates were below the horizontal plane or inside walls,
you would die immediately. Dimension now persists across reloads.

### Bucket

- Empty bucket raycast now ignores "skip liquid" mode; right-clicking a water surface will actually hit the water surface rather than the sand block below
- If the ray hits a solid block adjacent to water, the bucket falls back to the face-adjacent cell and picks up the liquid there
- Bucket placement places the **dynamic** fluid id (water 8, lava 10) and schedules a tick to let it actually start flowing

### Start Screen

Removed the Kolyah35 credits text, the GitHub icon blit, and the clickable URL handler
from `StartMenuScreen` and `TouchStartMenuScreen`.

### Windows 2000 / XP Compatibility

OpenGL context request lowered from 2.1 to 1.1 (`main_glfw.h`), so that Mesa3D's software
rasterizer (which the Win2K extended kernel VM will eventually use) can satisfy the request.
A 32-bit, self-contained Mesa3D 17.3.7 `opengl32.dll` is checked in under `win2k/`; on
systems where the native OpenGL driver is inadequate, copy it next to `MinecraftPE.exe`.
Newer Mesa branches (20.x, 26.x) cannot load on Win2K with the extended kernel — 17.3.7 is
the last version `pal1000` explicitly built for the old Windows ABI.

## Build

Supported build environment: **Windows + Visual Studio 2017 Build Tools**, using the
`v141_xp` toolset. The PowerShell script handles the vcvars environment, selects the
corresponding Ninja, and outputs the program to `build-xp\MinecraftPE.exe`.

```powershell
# Run from the repository root
powershell.exe -ExecutionPolicy Bypass -File .\build-xp.ps1

# Clean and rebuild
.\build-xp.ps1 -Clean
```

**Dependencies:**

- Installed Visual Studio 2017 build tools with the `v141_xp` (Windows XP) toolset. The script explicitly uses `-vcvars_ver=14.16` to prevent newer toolsets from being silently called. 
- CMake 3.21 or later. 
- Ninja added to the system `PATH` environment variable (you can also modify the `$NinjaDir` parameter in `build-xp.ps1`). 
- Git (some CMake `FetchContent` dependencies will be cloned during the configuration phase). 
The build output file is `build-xpMinecraftPE.exe`. The `data` folder next to the .exe holds game assets (terrain.png, items.png, language files, etc.) — the build process automatically copies these assets into `build-xpdata`.

### Running on Windows 2000 (Extended Kernel)
1. Follow the steps above to complete the build.
2. Copy `build-xpMinecraftPE.exe` and the `data` folder next to it to the target machine.
3. Copy `win2kopengl32.dll` (about 24MB) to the directory containing MinecraftPE.exe. **Do not** overwrite `C:WINNTsystem32opengl32.dll` — keep the Mesa version in the program directory, it only affects this game.
4. Optional: before launching, set the environment variable `set GALLIUM_DRIVER=softpipe` to force using a pure C software renderer. By default, llvmpipe uses JIT rendering and requires SSE2 instructions; use this option if you encounter compatibility issues.

## Controls
| Key | Function |
| --- | --- |
| W A S D | Move |
| Mouse | Look around |
| Left Mouse Button | Break blocks |
| Right Mouse Button | Place blocks |
| Space | Jump |
| Shift | Sneak |
| E | Open inventory |
| ESC | Pause / Exit |
| F1 | Hide HUD |
| F3 | Debug info (FPS, coordinates) |

> Mouse capture: After running the game, the mouse will be captured by the window; press Alt+Tab to release the mouse.

## Known Bugs So Far

1. Music playback occasionally stutters, looping issues (DirectSound audio layer port defect)
2. Some particle effects render incorrectly (fire, explosion particles)
3. Water rendering has flaws, transparent blocks have Z-fighting (texture flicker overlap)
4. Save compatibility: can only read PE 0.6.1 original saves; higher version saves can't be loaded
5. Occasionally quitting the game causes memory leaks and crashes
6. On XP systems, audio initialization fails more often

## Dev Notes

| Issue | Solution |
| --- | --- |
| Changed `.h` without `-Clean` → `0xc0000005` | Must `-Clean` after changing `.h` |
| CMake fails to download zlib during `-Clean` | Use a proxy / VPN |
| `OreFeature::place()` recursion in Level crashes | Modify `blocks[]` array directly |
| `git push` rejected | Run `git pull origin main` before push |
| Option automatically adds `options.` prefix | Write key as `language` (no prefix) |
| `renderGuiItemDecorations` durability bar gets overwritten | Draw order: icon → mouse cursor → durability bar |
| `Tesselator` undefined | `#include "../../../renderer/Tesselator.h"` |
| `const char* x = I18n::get(...).c_str()` dangling pointer | Change to `std::string x = I18n::get(...)` |
| Static `I18n::get()` executed before `main()` causes crash | Use lazy initialization inside function or hardcode |

## Thanks

- Original project: [JackTulli/Minecraft-PE-0.6.1-Win32-port](https://github.com/JackTulli/Minecraft-PE-0.6.1-Win32-port)
- Fishing rod reference: 4J Studios console version implementation
- Multilanguage system reference: [LamPbulB](https://github.com/coj211/MCCE)

## License

This project is for learning and research purposes only. Minecraft is a trademark of Mojang AB, and this project is not associated with Mojang.