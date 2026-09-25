#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__LeverTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__LeverTile_H__

#include "Tile.h"
#include "../Level.h"
#include "../material/Material.h"
#include "../../entity/player/Player.h"
#include "../../item/Item.h"

// AuxValue layout (same idea as TorchTile):
//   bits 0..2 (0x7): mount face — 1:-X, 2:+X, 3:-Z, 4:+Z, 5:floor (-Y)
//   bit  3   (0x8): on/off state
// Right-clicking flips bit 0x8; mount comes from the placement face. We
// render as a small wood/cobble block protruding from the supporting
// face (MCPE 0.6.1's atlas has no proper lever sprite).
class LeverTile: public Tile
{
    typedef Tile super;
public:
    LeverTile(int id, int tex)
    :   super(id, tex, Material::decoration)
    {
        // Default to floor-mount geometry; updateShape() switches to the
        // correct per-mount shape once the tile has data in the world.
        setShape(5/16.0f, 0, 5/16.0f, 11/16.0f, 10/16.0f, 11/16.0f);
    }

    AABB* getAABB(Level*, int, int, int) { return NULL; }
    bool  isCubeShaped()                 { return false; }
    bool  isSolidRender()                { return false; }

    // ON = cobblestone block, OFF = planks. Same for every face — we
    // don't have a proper lever sprite in this build's terrain.png so
    // we use the whole-cube texture to signal state.
    int getTexture(LevelSource* level, int x, int y, int z, int /*face*/) {
        return ((level->getData(x, y, z) & 0x8) != 0) ? (6 + 0 * 16) : tex;
    }

    bool mayPlace(Level* level, int x, int y, int z) {
        return validMountFor(level, x, y, z) != 0;
    }

    // Map clicked-face → mount value, mirroring TorchTile's convention.
    // (face here is the face of the block that was clicked; we've
    // already been moved into the adjacent cell by TileItem::useOn.)
    int getPlacedOnFaceDataValue(Level* level, int x, int y, int z,
                                  int face, float, float, float, int itemValue) {
        int dir = itemValue & 0x7;
        if (face == 1 && level->isSolidBlockingTile(x, y - 1, z)) dir = 5;
        else if (face == 2 && level->isSolidBlockingTile(x, y, z + 1)) dir = 4;
        else if (face == 3 && level->isSolidBlockingTile(x, y, z - 1)) dir = 3;
        else if (face == 4 && level->isSolidBlockingTile(x + 1, y, z)) dir = 2;
        else if (face == 5 && level->isSolidBlockingTile(x - 1, y, z)) dir = 1;
        return dir;
    }

    void onPlace(Level* level, int x, int y, int z) {
        int oldData = level->getData(x, y, z);
        int state = oldData & 0x8;
        int mount = oldData & 0x7;
        // If the mount we were placed with doesn't actually have a
        // support anymore (or it's zero from a /setblock), pick the
        // first available face. validMountFor returns 0 if none — in
        // that case we drop ourselves.
        if (!hasSupport(level, x, y, z, mount)) {
            mount = validMountFor(level, x, y, z);
        }
        if (mount == 0) {
            spawnResources(level, x, y, z, oldData);
            level->setTile(x, y, z, 0);
            return;
        }
        level->setData(x, y, z, mount | state);
    }

    bool use(Level* level, int x, int y, int z, Player* /*player*/) {
        int data = level->getData(x, y, z);
        int newData = data ^ 0x8;
        level->setData(x, y, z, newData);
        level->setTilesDirty(x, y, z, x, y, z);
        level->updateNeighborsAt(x, y, z, this->id);
        // Nudge the actual support block too so things attached to it
        // (other levers, redstone torches on a different face of the
        // same support, etc.) re-evaluate.
        int mount = newData & 0x7;
        int sx, sy, sz;
        supportPos(x, y, z, mount, sx, sy, sz);
        level->updateNeighborsAt(sx, sy, sz, this->id);
        return true;
    }

    bool isSignalSource() { return true; }

    // Weak power: any face outward (vanilla: weak power in all dirs).
    bool getSignal(LevelSource* level, int x, int y, int z, int /*dir*/) {
        return (level->getData(x, y, z) & 0x8) != 0;
    }

    // Strong power: only into the block we're attached to (vanilla rule).
    bool getDirectSignal(Level* level, int x, int y, int z, int dir) {
        int data = level->getData(x, y, z);
        if (!(data & 0x8)) return false;
        int mount = data & 0x7;
        return dir == supportDirFromReceiver(mount);
    }

    void neighborChanged(Level* level, int x, int y, int z, int /*type*/) {
        int data = level->getData(x, y, z);
        int mount = data & 0x7;
        if (!hasSupport(level, x, y, z, mount)) {
            spawnResources(level, x, y, z, data);
            level->setTile(x, y, z, 0);
        }
    }

    // Render path also calls updateShape before drawing, so the shape
    // member variables stay in sync with whatever mount we're on.
    void updateShape(LevelSource* level, int x, int y, int z) {
        setShapeFor(level->getData(x, y, z) & 0x7);
    }

private:
    static int supportDirFromReceiver(int mount) {
        switch (mount) {
            case 1: return 5;  // support at -X; receiver looks +X (east) to find us
            case 2: return 4;
            case 3: return 3;
            case 4: return 2;
            case 5: default: return 1;
        }
    }

    static void supportPos(int x, int y, int z, int mount, int& sx, int& sy, int& sz) {
        sx = x; sy = y; sz = z;
        switch (mount) {
            case 1: sx -= 1; break;
            case 2: sx += 1; break;
            case 3: sz -= 1; break;
            case 4: sz += 1; break;
            case 5: default: sy -= 1; break;
        }
    }

    static bool hasSupport(Level* level, int x, int y, int z, int mount) {
        if (mount < 1 || mount > 5) return false;
        int sx, sy, sz;
        supportPos(x, y, z, mount, sx, sy, sz);
        if (mount == 5) {
            int below = level->getTile(sx, sy, sz);
            if (below == 0) return false;
            Tile* t = Tile::tiles[below];
            return t && t->isSolidRender();
        }
        return level->isSolidBlockingTile(sx, sy, sz);
    }

    static int validMountFor(Level* level, int x, int y, int z) {
        if (level->isSolidBlockingTile(x - 1, y, z)) return 1;
        if (level->isSolidBlockingTile(x + 1, y, z)) return 2;
        if (level->isSolidBlockingTile(x, y, z - 1)) return 3;
        if (level->isSolidBlockingTile(x, y, z + 1)) return 4;
        int below = level->getTile(x, y - 1, z);
        if (below > 0 && Tile::tiles[below] && Tile::tiles[below]->isSolidRender()) {
            return 5;
        }
        return 0;
    }

    void setShapeFor(int mount) {
        switch (mount) {
            case 1:  // attached to wall at -X; body protrudes into +X
                setShape(0,         4/16.0f, 5/16.0f, 6/16.0f,  12/16.0f, 11/16.0f);
                break;
            case 2:  // attached at +X
                setShape(10/16.0f,  4/16.0f, 5/16.0f, 1,        12/16.0f, 11/16.0f);
                break;
            case 3:  // attached at -Z
                setShape(5/16.0f,   4/16.0f, 0,       11/16.0f, 12/16.0f, 6/16.0f);
                break;
            case 4:  // attached at +Z
                setShape(5/16.0f,   4/16.0f, 10/16.0f, 11/16.0f, 12/16.0f, 1);
                break;
            case 5:
            default:
                setShape(5/16.0f,   0,       5/16.0f, 11/16.0f, 10/16.0f, 11/16.0f);
                break;
        }
    }
};

#endif
