#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__PortalTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__PortalTile_H__

#include "Tile.h"
#include "../Level.h"
#include "../material/Material.h"

// Simple Nether portal block. Non-solid (player walks through), faintly
// glowing, drops nothing when broken. Standing in one for a number of ticks
// triggers a dimension switch (handled in LocalPlayer::tick).
//
// A static helper, tryLight(), implements obsidian-frame detection: given a
// clicked air position, checks for a vertical obsidian rectangle (any axis,
// minimum 2x3 interior) surrounding the click and fills the interior with
// portal blocks. Called from FlintAndSteelItem.
class PortalTile : public Tile
{
public:
    PortalTile(int id, int tex)
    :   Tile(id, tex, Material::air)
    {
    }

    bool isCubeShaped() { return false; }
    bool isSolidRender() { return false; }
    bool mayPick()       { return false; }
    bool mayPlace(Level*, int, int, int) { return false; }

    AABB* getAABB(Level* level, int x, int y, int z) {
        // Non-colliding — return null so entities pass through freely.
        return NULL;
    }

    int getResourceCount(Random* random) { return 0; }
    int getResource(int data, Random* random) { return 0; }

    // Attempts to light a portal anchored at the clicked air block at (x,y,z).
    // Searches for a valid obsidian frame around it (both X and Z axis
    // orientations). If found, fills the interior with portal blocks and
    // returns true. Otherwise returns false (caller can fall through to
    // placing fire instead).
    static bool tryLight(Level* level, int x, int y, int z);

private:
    // Internal: try to find a frame on a specific orientation.
    // axisIsX = true  -> the portal plane spans X/Y, frame normal is Z.
    // axisIsX = false -> the portal plane spans Z/Y, frame normal is X.
    static bool tryLightOnAxis(Level* level, int x, int y, int z, bool axisIsX);
};

#endif
