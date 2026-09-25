#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__NetherFenceTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__NetherFenceTile_H__

#include "FenceTile.h"
#include "../material/Material.h"

// Nether brick fence. Identical geometry to a wooden fence but stone-class
// material (so it doesn't burn / break with a hatchet faster than a pick)
// and uses the netherBrick atlas tile. Inherits FenceTile::connectsTo, so
// nether fences only connect to other nether fences and fence gates — never
// to wooden fences (vanilla Java behaviour pre-1.16).
class NetherFenceTile: public FenceTile
{
public:
    NetherFenceTile(int id)
    :   FenceTile(id, 0 + 14 * 16 /* netherBrick texture */, Material::stone)
    {
    }
};

#endif
