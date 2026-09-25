#include "PortalTile.h"
#include "Tile.h"
#include "../Level.h"

namespace {
// Maximum interior dimensions for a valid frame, in blocks.
// Java uses 21x21 max; we follow the same limits.
const int MAX_INNER = 21;

inline bool isObsidian(Level* level, int x, int y, int z) {
    int t = level->getTile(x, y, z);
    return t == Tile::obsidian->id;
}

inline bool isAirOrPortal(Level* level, int x, int y, int z) {
    int t = level->getTile(x, y, z);
    if (t == 0) return true;
    if (Tile::portalTile && t == Tile::portalTile->id) return true;
    return false;
}
}

bool PortalTile::tryLight(Level* level, int x, int y, int z) {
    if (!Tile::obsidian || !Tile::portalTile) return false;

    // Caller has confirmed (x,y,z) is currently air. Try both axes.
    if (tryLightOnAxis(level, x, y, z, true)) return true;
    if (tryLightOnAxis(level, x, y, z, false)) return true;
    return false;
}

bool PortalTile::tryLightOnAxis(Level* level, int x, int y, int z, bool axisIsX) {
    // For axisIsX=true: portal plane spans X and Y; Z is fixed at z.
    // For axisIsX=false: portal plane spans Z and Y; X is fixed at x.

    // Walk down to find the floor of the frame: scan downward from y until we
    // hit obsidian. That's the row just below the interior.
    int floorY = y;
    while (floorY > 0 && !isObsidian(level, x, floorY - 1, z) &&
           isAirOrPortal(level, x, floorY - 1, z))
    {
        --floorY;
    }
    if (floorY <= 0) return false;
    if (!isObsidian(level, x, floorY - 1, z)) return false;

    // Walk up to find the ceiling row.
    int ceilY = y;
    while (ceilY < 126 && !isObsidian(level, x, ceilY + 1, z) &&
           isAirOrPortal(level, x, ceilY + 1, z))
    {
        ++ceilY;
    }
    if (!isObsidian(level, x, ceilY + 1, z)) return false;

    int innerH = ceilY - floorY + 1;
    if (innerH < 3 || innerH > MAX_INNER) return false;

    // Walk left along the active axis to find the left edge.
    int minA, maxA;
    int a0 = axisIsX ? x : z;
    int minA_search = a0;
    while (minA_search > 0) {
        int ax = axisIsX ? minA_search - 1 : x;
        int az = axisIsX ? z : minA_search - 1;
        if (isObsidian(level, ax, floorY, az)) break;
        if (!isAirOrPortal(level, ax, floorY, az)) return false;
        --minA_search;
    }
    if (!isObsidian(level, axisIsX ? minA_search - 1 : x,
                           floorY,
                           axisIsX ? z : minA_search - 1)) return false;
    minA = minA_search;

    // Walk right to find right edge.
    int maxA_search = a0;
    while (maxA_search < (axisIsX ? 254 : 254)) {
        int ax = axisIsX ? maxA_search + 1 : x;
        int az = axisIsX ? z : maxA_search + 1;
        if (isObsidian(level, ax, floorY, az)) break;
        if (!isAirOrPortal(level, ax, floorY, az)) return false;
        ++maxA_search;
    }
    if (!isObsidian(level, axisIsX ? maxA_search + 1 : x,
                           floorY,
                           axisIsX ? z : maxA_search + 1)) return false;
    maxA = maxA_search;

    int innerW = maxA - minA + 1;
    if (innerW < 2 || innerW > MAX_INNER) return false;

    // Validate the entire rectangle:
    //  - floor row (y = floorY - 1) and ceiling row (y = ceilY + 1) are all obsidian
    //  - left column (a = minA - 1) and right column (a = maxA + 1) are all obsidian
    //  - the interior is all air-or-portal
    for (int a = minA; a <= maxA; ++a) {
        int ax = axisIsX ? a : x;
        int az = axisIsX ? z : a;
        if (!isObsidian(level, ax, floorY - 1, az)) return false;
        if (!isObsidian(level, ax, ceilY + 1, az))  return false;
    }
    for (int yy = floorY; yy <= ceilY; ++yy) {
        int leftAX  = axisIsX ? minA - 1 : x;
        int leftAZ  = axisIsX ? z : minA - 1;
        int rightAX = axisIsX ? maxA + 1 : x;
        int rightAZ = axisIsX ? z : maxA + 1;
        if (!isObsidian(level, leftAX,  yy, leftAZ))  return false;
        if (!isObsidian(level, rightAX, yy, rightAZ)) return false;
    }
    for (int a = minA; a <= maxA; ++a)
    for (int yy = floorY; yy <= ceilY; ++yy) {
        int ax = axisIsX ? a : x;
        int az = axisIsX ? z : a;
        if (!isAirOrPortal(level, ax, yy, az)) return false;
    }

    // Frame is valid — fill interior with portal blocks.
    int portalId = Tile::portalTile->id;
    for (int a = minA; a <= maxA; ++a)
    for (int yy = floorY; yy <= ceilY; ++yy) {
        int ax = axisIsX ? a : x;
        int az = axisIsX ? z : a;
        level->setTile(ax, yy, az, portalId);
    }
    return true;
}
