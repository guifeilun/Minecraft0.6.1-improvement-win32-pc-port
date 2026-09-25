#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__PumpkinTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__PumpkinTile_H__

#include "Tile.h"
#include "../material/Material.h"
#include "../../Facing.h"
#include "../../entity/Mob.h"
#include "../Level.h"
#include "../../../util/Mth.h"

// Pumpkin / jack-o'-lantern. Six-sided cube with a carved face on one
// side, oriented to face the placer. `lit=true` swaps the face texture
// for the lit variant and emits light. The pumpkin->torch upgrade to
// the lit form is handled by FlintAndSteelItem (or the matching
// crafting recipe of pumpkin + torch) rather than this class.
class PumpkinTile: public Tile
{
    typedef Tile super;
public:
    PumpkinTile(int id, bool lit)
    :   super(id, 6 + 7 * 16 /* pumpkin side */, Material::vegetable),
        _lit(lit)
    {
    }

    bool isLit() const { return _lit; }

    int getTexture(int face) {
        // No data context (inventory icon): show top and one face only.
        if (face == 1) return 6 + 6 * 16;        // pumpkin top
        if (face == 0) return 6 + 6 * 16;        // pumpkin bottom — reuse top, no separate bottom in atlas
        if (face == 3) return (_lit ? 8 : 7) + 7 * 16; // south = front
        return 6 + 7 * 16;                       // side
    }

    int getTexture(LevelSource* level, int x, int y, int z, int face) {
        if (face == 1 || face == 0) return 6 + 6 * 16;
        int dir = level->getData(x, y, z);
        if (face == dir) return (_lit ? 8 : 7) + 7 * 16;
        return 6 + 7 * 16;
    }

    void setPlacedBy(Level* level, int x, int y, int z, Mob* by) {
        if (!by) return;
        // Place the face toward the player (i.e. opposite the direction
        // they're looking). Map yaw onto NORTH/EAST/SOUTH/WEST in the
        // same way FurnaceTile does.
        int dir = (Mth::floor(by->yRot * 4 / 360 + 0.5f)) & 3;
        int face = Facing::NORTH;
        if (dir == 1) face = Facing::EAST;
        if (dir == 2) face = Facing::SOUTH;
        if (dir == 3) face = Facing::WEST;
        level->setData(x, y, z, face);
    }

private:
    bool _lit;
};

#endif
