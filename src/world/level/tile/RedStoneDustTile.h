#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__RedStoneDustTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__RedStoneDustTile_H__

#include "Tile.h"
#include "../Level.h"
#include "../material/Material.h"
#include "../../../util/Random.h"
#include "../../item/Item.h"

// Redstone wire. Stores a 0..15 strength in auxValue (vanilla layout).
// Propagation rule per Level::updateNeighborsAt -> neighborChanged:
//   our_new_strength = max over the 6 neighbours of:
//     - if neighbour is another wire: neighbour.strength - 1
//     - if neighbour emits any signal (via Level::getSignal, which also
//       handles "solid block adjacent to a direct source"): 15
// If our_new_strength differs from our current strength we update aux and
// propagate neighborChanged to all 6 neighbours. This converges in
// O(MAX_STRENGTH) hops in the worst case because each step the strength
// can only move toward the steady-state value.
//
// The grayscale cross sprite at (4,10) is tinted red at render time via
// getColor(); brighter for higher strength.
class RedStoneDustTile: public Tile
{
    typedef Tile super;
public:
    static const int MAX_STRENGTH = 15;

    RedStoneDustTile(int id, int tex)
    :   super(id, tex, Material::decoration)
    {
        setShape(0, 0, 0, 1, 1.0f / 16.0f, 1);
    }

    AABB* getAABB(Level*, int, int, int) { return NULL; }
    bool  isCubeShaped()                 { return false; }
    bool  isSolidRender()                { return false; }
    bool  isSignalSource()               { return true; }

    bool mayPlace(Level* level, int x, int y, int z) {
        int below = level->getTile(x, y - 1, z);
        if (below == 0) return false;
        Tile* t = Tile::tiles[below];
        return t && t->isSolidRender();
    }

    // Weak power: any neighbour that asks gets our strength as a boolean.
    bool getSignal(LevelSource* level, int x, int y, int z, int /*dir*/) {
        return level->getData(x, y, z) > 0;
    }

    // Strong power: wires only strongly power the block directly beneath
    // them. dir==1 means the caller is below us (looking up to reach the
    // wire). Without this restriction wires would directly power adjacent
    // blocks, which then propagate through hasDirectSignal and let signal
    // hop sideways through solid blocks — not how vanilla works.
    bool getDirectSignal(Level* level, int x, int y, int z, int dir) {
        return dir == 1 && level->getData(x, y, z) > 0;
    }

    // Grayscale sprite → red tint. Dim red when unpowered, bright red at
    // full strength. Players can read approximate signal strength from
    // the brightness now.
    int getColor(LevelSource* level, int x, int y, int z) {
        int s = level->getData(x, y, z);
        if (s == 0) return 0x600000;
        // map 1..15 → 0x80..0xFF on the red channel
        int r = 0x80 + (s * 0x7f) / MAX_STRENGTH;
        if (r > 0xff) r = 0xff;
        return (r << 16) | 0x10;
    }

    void onPlace(Level* level, int x, int y, int z) {
        updateSignal(level, x, y, z);
    }

    void neighborChanged(Level* level, int x, int y, int z, int /*type*/) {
        // Drop the wire if its supporting block disappears.
        int below = level->getTile(x, y - 1, z);
        Tile* bt = (below > 0) ? Tile::tiles[below] : NULL;
        if (!bt || !bt->isSolidRender()) {
            spawnResources(level, x, y, z, level->getData(x, y, z));
            level->setTile(x, y, z, 0);
            return;
        }
        updateSignal(level, x, y, z);
    }

    int getResource(int /*data*/, Random* /*random*/) { return Item::redStone->id; }
    int getResourceCount(Random*)                      { return 1; }

private:
    static void offset(int d, int x, int y, int z, int& nx, int& ny, int& nz) {
        nx = x; ny = y; nz = z;
        switch (d) {
            case 0: --ny; break;
            case 1: ++ny; break;
            case 2: --nz; break;
            case 3: ++nz; break;
            case 4: --nx; break;
            case 5: ++nx; break;
        }
    }

    void updateSignal(Level* level, int x, int y, int z) {
        int oldStrength = level->getData(x, y, z);

        // Clear ourselves first so neighbour wires that scan us during
        // this update see our cleared state — keeps a single update wave
        // from feeding itself back through us as a stale source.
        bool prevSuppress = level->noNeighborUpdate;
        level->noNeighborUpdate = true;
        level->setData(x, y, z, 0);

        int newStrength = 0;
        for (int d = 0; d < 6; ++d) {
            int nx, ny, nz;
            offset(d, x, y, z, nx, ny, nz);

            int nt = level->getTile(nx, ny, nz);
            if (nt == 0) continue;

            if (nt == this->id) {
                int s = level->getData(nx, ny, nz);
                int contribute = s - 1;
                if (contribute > newStrength) newStrength = contribute;
            } else {
                // Non-wire neighbour. Level::getSignal handles both
                // direct emitters (lever/torch/plate/button) and
                // weak-power-through-solid-blocks (a stone with a lever
                // on top, queried from the side).
                if (level->getSignal(nx, ny, nz, d)) {
                    newStrength = MAX_STRENGTH;
                }
            }
        }
        if (newStrength > MAX_STRENGTH) newStrength = MAX_STRENGTH;
        if (newStrength < 0)            newStrength = 0;

        level->setData(x, y, z, newStrength);
        level->noNeighborUpdate = prevSuppress;

        if (newStrength != oldStrength) {
            level->setTilesDirty(x, y, z, x, y, z);

            // First wave: the 6 cells directly adjacent to us — wires
            // recompute, doors read hasNeighborSignal, TNT primes, etc.
            for (int d = 0; d < 6; ++d) {
                int nx, ny, nz;
                offset(d, x, y, z, nx, ny, nz);
                int nt = level->getTile(nx, ny, nz);
                if (nt > 0 && Tile::tiles[nt]) {
                    Tile::tiles[nt]->neighborChanged(level, nx, ny, nz, this->id);
                }
            }

            // Second wave: the neighbours-of-neighbours. A redstone torch
            // attached to a block that's adjacent to us has no idea its
            // support's weak-power state just changed unless we knock on
            // its door directly — the block in between is just a passive
            // relay (Tile::neighborChanged is a no-op). Java does the
            // same "indirect neighbour update" for the weak-power model.
            // Skip ourselves to avoid a redundant recompute.
            for (int d = 0; d < 6; ++d) {
                int nx, ny, nz;
                offset(d, x, y, z, nx, ny, nz);
                for (int d2 = 0; d2 < 6; ++d2) {
                    int mx, my, mz;
                    offset(d2, nx, ny, nz, mx, my, mz);
                    if (mx == x && my == y && mz == z) continue;
                    int mt = level->getTile(mx, my, mz);
                    if (mt > 0 && Tile::tiles[mt]) {
                        Tile::tiles[mt]->neighborChanged(level, mx, my, mz, this->id);
                    }
                }
            }
        }
    }
};

#endif
