#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__RailTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__RailTile_H__

#include "Tile.h"
#include "../Level.h"
#include "../material/Material.h"

// Rail tiles (regular / powered / detector). Cosmetic-only for now: they
// place as flat 1/16-thick slabs with the rail texture on top, but there's
// no minecart entity or curve/slope detection — the auxValue that vanilla
// uses to encode orientation is left at 0, so every rail looks like a
// straight piece running along its placed direction.
//
// Without a redstone signal system the "powered" variant of golden rail
// stays at its un-powered texture; without an entity-detection system the
// detector rail doesn't emit a signal. Both are stubbed in for visual
// completeness and recipe-availability.
class RailTile: public Tile
{
    typedef Tile super;
public:
    RailTile(int id, int tex)
    :   super(id, tex, Material::decoration)
    {
        // 1/16-thick flat slab — same trick TopSnowTile uses for a paper-thin
        // tile. Side faces are tiny strips of the rail texture but that's
        // mostly invisible against the ground.
        setShape(0, 0, 0, 1, 1.0f / 16.0f, 1);
    }

    AABB* getAABB(Level*, int, int, int)  { return NULL; }
    bool  blocksLight()                    { return false; }
    bool  isSolidRender()                  { return false; }
    bool  isCubeShaped()                   { return false; }

    bool mayPlace(Level* level, int x, int y, int z) {
        int below = level->getTile(x, y - 1, z);
        if (below == 0) return false;
        Tile* t = Tile::tiles[below];
        return t && t->isSolidRender();
    }

    void neighborChanged(Level* level, int x, int y, int z, int /*type*/) {
        // If the supporting block disappears, drop the rail.
        int below = level->getTile(x, y - 1, z);
        Tile* bt = (below > 0) ? Tile::tiles[below] : NULL;
        if (!bt || !bt->isSolidRender()) {
            spawnResources(level, x, y, z, level->getData(x, y, z));
            level->setTile(x, y, z, 0);
        }
    }
};

#endif
