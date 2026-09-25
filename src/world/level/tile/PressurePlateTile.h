#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__PressurePlateTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__PressurePlateTile_H__

#include "Tile.h"
#include "../Level.h"
#include "../material/Material.h"
#include "../../phys/AABB.h"
#include "../../../util/Random.h"
#include "../../entity/Entity.h"

// Stone / wood pressure plate. Random-ticks itself to scan for entities
// standing on top, sets auxValue bit 0x1 while pressed, emits a redstone
// signal in all directions when pressed.
class PressurePlateTile: public Tile
{
    typedef Tile super;
public:
    PressurePlateTile(int id, int tex, const Material* material)
    :   super(id, tex, material)
    {
        setShape(1.0f/16.0f, 0, 1.0f/16.0f, 15.0f/16.0f, 1.0f/16.0f, 15.0f/16.0f);
        setTicking(true);
    }

    AABB* getAABB(Level*, int, int, int) { return NULL; }
    bool  isCubeShaped()                 { return false; }
    bool  isSolidRender()                { return false; }
    bool  isSignalSource()               { return true; }

    bool getSignal(LevelSource* level, int x, int y, int z, int /*dir*/) {
        return level->getData(x, y, z) > 0;
    }

    bool getDirectSignal(Level* level, int x, int y, int z, int /*dir*/) {
        return level->getData(x, y, z) > 0;
    }

    bool mayPlace(Level* level, int x, int y, int z) {
        int below = level->getTile(x, y - 1, z);
        if (below == 0) return false;
        Tile* t = Tile::tiles[below];
        return t && t->isSolidRender();
    }

    void tick(Level* level, int x, int y, int z, Random*) {
        recheckPressed(level, x, y, z);
        // Re-schedule next check so we keep polling even after entities
        // leave. setTicking(true) buys us random ticks but they're sparse,
        // so we also queue a fixed-delay tick.
        level->addToTickNextTick(x, y, z, this->id, 10);
    }

    void onPlace(Level* level, int x, int y, int z) {
        level->addToTickNextTick(x, y, z, this->id, 10);
    }

    // Vanilla also re-checks on entityInside(), but we don't have that hook
    // wired; the periodic tick is good enough for now.

    void neighborChanged(Level* level, int x, int y, int z, int /*type*/) {
        int below = level->getTile(x, y - 1, z);
        Tile* bt = (below > 0) ? Tile::tiles[below] : NULL;
        if (!bt || !bt->isSolidRender()) {
            spawnResources(level, x, y, z, level->getData(x, y, z));
            level->setTile(x, y, z, 0);
        }
    }

private:
    void recheckPressed(Level* level, int x, int y, int z) {
        int oldData = level->getData(x, y, z);

        // AABB extending slightly above the plate's hitbox; vanilla uses
        // (0.125, 0, 0.125)..(0.875, 0.25, 0.875). Any entity intersecting
        // counts as pressing.
        AABB pressBB(
            (float)x + 0.125f, (float)y, (float)z + 0.125f,
            (float)x + 0.875f, (float)y + 0.25f, (float)z + 0.875f);

        EntityList& ents = level->getEntities(NULL, pressBB);
        int newData = ents.empty() ? 0 : 1;

        if (newData != oldData) {
            level->setData(x, y, z, newData);
            level->setTilesDirty(x, y, z, x, y, z);
            level->updateNeighborsAt(x, y, z, this->id);
            level->updateNeighborsAt(x, y - 1, z, this->id);
        }
    }
};

#endif
