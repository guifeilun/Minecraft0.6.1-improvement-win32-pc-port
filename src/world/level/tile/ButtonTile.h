#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__ButtonTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__ButtonTile_H__

#include "Tile.h"
#include "../Level.h"
#include "../material/Material.h"
#include "../../entity/player/Player.h"
#include "../../../util/Random.h"

// AuxValue layout matches LeverTile:
//   bits 0..2 (0x7): mount face — 1:-X, 2:+X, 3:-Z, 4:+Z, 5:floor
//   bit  3   (0x8): pressed
// Pressing sets bit 0x8 and schedules a tick `_pressDelay` ticks out;
// the tick clears the bit. Stone buttons hold for 20 ticks, wood for 30.
class ButtonTile: public Tile
{
    typedef Tile super;
public:
    ButtonTile(int id, int tex, const Material* material)
    :   super(id, tex, material),
        _pressDelay(material == Material::wood ? 30 : 20)
    {
        setTicking(true);
        setShape(5/16.0f, 0, 5/16.0f, 11/16.0f, 6/16.0f, 11/16.0f);
    }

    AABB* getAABB(Level*, int, int, int) { return NULL; }
    bool  isCubeShaped()                 { return false; }
    bool  isSolidRender()                { return false; }

    bool mayPlace(Level* level, int x, int y, int z) {
        return validMountFor(level, x, y, z) != 0;
    }

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
        if (data & 0x8) return true;  // already pressed
        level->setData(x, y, z, data | 0x8);
        level->setTilesDirty(x, y, z, x, y, z);
        level->updateNeighborsAt(x, y, z, this->id);
        int mount = data & 0x7;
        int sx, sy, sz;
        supportPos(x, y, z, mount, sx, sy, sz);
        level->updateNeighborsAt(sx, sy, sz, this->id);
        level->addToTickNextTick(x, y, z, this->id, _pressDelay);
        return true;
    }

    void tick(Level* level, int x, int y, int z, Random* /*random*/) {
        int data = level->getData(x, y, z);
        if (!(data & 0x8)) return;
        level->setData(x, y, z, data & ~0x8);
        level->setTilesDirty(x, y, z, x, y, z);
        level->updateNeighborsAt(x, y, z, this->id);
        int mount = data & 0x7;
        int sx, sy, sz;
        supportPos(x, y, z, mount, sx, sy, sz);
        level->updateNeighborsAt(sx, sy, sz, this->id);
    }

    bool isSignalSource() { return true; }

    bool getSignal(LevelSource* level, int x, int y, int z, int /*dir*/) {
        return (level->getData(x, y, z) & 0x8) != 0;
    }

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

    void updateShape(LevelSource* level, int x, int y, int z) {
        setShapeFor(level->getData(x, y, z) & 0x7);
    }

private:
    int _pressDelay;

    static int supportDirFromReceiver(int mount) {
        switch (mount) {
            case 1: return 5;
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
        // Smaller than the lever — buttons are flatter against the wall.
        switch (mount) {
            case 1:
                setShape(0,         5/16.0f, 6/16.0f, 2/16.0f,  11/16.0f, 10/16.0f);
                break;
            case 2:
                setShape(14/16.0f,  5/16.0f, 6/16.0f, 1,        11/16.0f, 10/16.0f);
                break;
            case 3:
                setShape(6/16.0f,   5/16.0f, 0,       10/16.0f, 11/16.0f, 2/16.0f);
                break;
            case 4:
                setShape(6/16.0f,   5/16.0f, 14/16.0f, 10/16.0f, 11/16.0f, 1);
                break;
            case 5:
            default:
                setShape(5/16.0f,   0,       5/16.0f, 11/16.0f, 2/16.0f, 11/16.0f);
                break;
        }
    }
};

#endif
