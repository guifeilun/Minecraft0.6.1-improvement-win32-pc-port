#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__RedstoneLampTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__RedstoneLampTile_H__

#include "Tile.h"
#include "../Level.h"
#include "../material/Material.h"
#include "../../../util/Random.h"

// Solid cube that swaps texture / light emission based on redstone input.
//   id 123 (redStoneLamp_off): dim, no light.
//   id 124 (redStoneLamp_on):  bright, light level 15.
// neighborChanged schedules a 2-tick check; tick() does the actual
// setTile swap. Both variants drop the off form so mining either gives
// you a placeable lamp item back.
class RedstoneLampTile: public Tile
{
    typedef Tile super;
public:
    RedstoneLampTile(int id, int tex, bool lit)
    :   super(id, tex, Material::wood),
        _lit(lit)
    {
        setTicking(true);
    }

    bool isSignalSource() { return false; }

    void onPlace(Level* level, int x, int y, int z) {
        super::onPlace(level, x, y, z);
        // Power state on placement should match neighbours immediately;
        // queue a check so a lamp placed next to a live wire lights up
        // (or an "on" lamp placed in an unpowered spot goes dark) within
        // 2 ticks.
        level->addToTickNextTick(x, y, z, this->id, 2);
    }

    void neighborChanged(Level* level, int x, int y, int z, int /*type*/) {
        if (shouldToggle(level, x, y, z)) {
            level->addToTickNextTick(x, y, z, this->id, 2);
        }
    }

    void tick(Level* level, int x, int y, int z, Random* /*random*/) {
        if (!shouldToggle(level, x, y, z)) return;

        Tile* swapTo = _lit ? Tile::redStoneLamp_off : Tile::redStoneLamp_on;
        if (!swapTo) return;

        // setTile preserves data (aux 0 for both lamp variants) and
        // fires updateNeighborsAt for the 6 direct neighbours, which
        // is enough for the lamp's visual / lighting refresh — the
        // lamp itself isn't a signal source so no second-hop fan-out
        // is required.
        level->setTile(x, y, z, swapTo->id);
    }

    // Both lit and unlit drop the unlit form so the item economy stays
    // single-id (we never expose an "on lamp" as an item).
    int getResource(int /*data*/, Random* /*random*/) {
        return Tile::redStoneLamp_off ? Tile::redStoneLamp_off->id : this->id;
    }

private:
    bool _lit;

    bool shouldToggle(Level* level, int x, int y, int z) {
        bool powered = level->hasDirectSignal(x, y, z)
                    || level->hasNeighborSignal(x, y, z);
        // Lit when ANY signal source neighbours us; unlit otherwise.
        return powered != _lit;
    }
};

#endif
