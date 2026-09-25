#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__PistonTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__PistonTile_H__

#include "Tile.h"
#include "../material/Material.h"
#include "../Level.h"
#include "../../Facing.h"
#include "../../entity/Mob.h"
#include "../../../util/Mth.h"

// Piston body. Placeholder — renders as a 6-face cube oriented toward the
// placer so the wooden "head" is visible, but doesn't extend or move blocks
// (no piston-extension entity or redstone signal to drive it). Sticky and
// regular variants differ only in the head-face texture.
class PistonTile: public Tile
{
    typedef Tile super;
public:
    PistonTile(int id, bool sticky)
    :   super(id, 12 + 6 * 16 /* piston side */, Material::stone),
        _sticky(sticky)
    {
    }

    // Inventory icon — assume the head faces "south" so the wooden face is
    // visible on face 3.
    int getTexture(int face) {
        if (face == 3) return (_sticky ? 10 : 11) + 6 * 16;   // head
        if (face == 2) return 13 + 6 * 16;                     // back
        return 12 + 6 * 16;                                    // side
    }

    int getTexture(LevelSource* level, int x, int y, int z, int face) {
        int dir = level->getData(x, y, z);
        if (face == dir)                       return (_sticky ? 10 : 11) + 6 * 16;
        if (face == Facing::OPPOSITE_FACING[dir])
            return 13 + 6 * 16;
        return 12 + 6 * 16;
    }

    void setPlacedBy(Level* level, int x, int y, int z, Mob* by) {
        if (!by) {
            level->setData(x, y, z, Facing::NORTH);
            return;
        }
        // Pick the face closest to the placer's view direction. Vertical
        // pitch dominates: if the player is looking sharply up or down,
        // the piston points up/down; otherwise it's the cardinal yaw.
        float pitch = by->xRot;
        if (pitch <= -50.0f) { level->setData(x, y, z, Facing::UP);   return; }
        if (pitch >=  50.0f) { level->setData(x, y, z, Facing::DOWN); return; }

        int dir = (Mth::floor(by->yRot * 4 / 360 + 0.5f)) & 3;
        if (dir == 0) level->setData(x, y, z, Facing::NORTH);
        if (dir == 1) level->setData(x, y, z, Facing::EAST);
        if (dir == 2) level->setData(x, y, z, Facing::SOUTH);
        if (dir == 3) level->setData(x, y, z, Facing::WEST);
    }

private:
    bool _sticky;
};

#endif
