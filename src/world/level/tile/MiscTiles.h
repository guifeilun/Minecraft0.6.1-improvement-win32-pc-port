#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__MiscTiles_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__MiscTiles_H__

#include "Tile.h"
#include "../Level.h"
#include "../material/Material.h"

// A grab-bag of cosmetic placeholder tiles. Each one fills out an item the
// vanilla game would expose but that we don't have full simulation logic
// for yet (no redstone signal, no piston extension entity, no enchanting
// system, no brewing system, no jukebox playback, etc.). They're all
// physically present and recipe-craftable but their gameplay behaviour is
// a no-op beyond placing as a block.

// Solid cube with distinct top/side/bottom textures (e.g. jukebox).
class SimpleCubeTile: public Tile
{
public:
    SimpleCubeTile(int id, int topTex, int sideTex, int bottomTex,
                   const Material* material)
    :   Tile(id, sideTex, material),
        _top(topTex), _side(sideTex), _bottom(bottomTex)
    {
    }

    int getTexture(int face) {
        if (face == 1) return _top;
        if (face == 0) return _bottom;
        return _side;
    }
    int getTexture(int face, int /*data*/) { return getTexture(face); }

private:
    int _top, _side, _bottom;
};

// Cosmetic tile-with-shape-of-cube but with a paper-thin AABB (lever, button,
// pressure-plate-style flat-on-floor entries — picked when full slab depth
// looks wrong but we don't want to model the real geometry).
class CosmeticFlatTile: public Tile
{
public:
    CosmeticFlatTile(int id, int tex, const Material* material)
    :   Tile(id, tex, material)
    {
        setShape(6/16.0f, 0, 6/16.0f, 10/16.0f, 4/16.0f, 10/16.0f);
    }
    AABB* getAABB(Level*, int, int, int) { return NULL; }
    bool  isCubeShaped()                 { return false; }
    bool  isSolidRender()                { return false; }
};

#endif
