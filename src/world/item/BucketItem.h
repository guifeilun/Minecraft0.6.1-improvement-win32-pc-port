#ifndef NET_MINECRAFT_WORLD_ITEM__BucketItem_H__
#define NET_MINECRAFT_WORLD_ITEM__BucketItem_H__

#include "Item.h"
#include "ItemInstance.h"
#include "../entity/player/Player.h"
#include "../level/Level.h"
#include "../level/tile/Tile.h"
#include "platform/log.h"

// Empty / water / lava buckets. The "contents" tile id is fixed at
// construction:
//   0                       -> empty (right-click picks up liquid)
//   Tile::calmWater->id     -> water (right-click places water, returns empty)
//   Tile::calmLava->id      -> lava  (right-click places lava,  returns empty)
//
// After a successful interaction the player's held ItemInstance is swapped
// in place to the resulting bucket type. We don't yet support cow-milking;
// that needs a Cow interact hook.
class BucketItem: public Item
{
    typedef Item super;
public:
    BucketItem(int id, int contentsTileId)
    :   super(id), _contents(contentsTileId)
    {
        setMaxStackSize(1);
    }

    bool useOn(ItemInstance* instance, Player* player, Level* level,
               int x, int y, int z, int face,
               float, float, float)
    {
        if (!instance || !level) return false;

        if (_contents == 0) {
            // Empty bucket: try to pick up liquid. If the directly-clicked
            // tile isn't a liquid, also look one block in the direction of
            // the clicked face — common case is the player on a beach,
            // looking down through a flowing-water surface, with the
            // raycast landing on sand. Vanilla wants a source block
            // (data == 0 or 8) but we accept any liquid here so a flowing
            // surface still hands the player a bucket of water.
            int t = level->getTile(x, y, z);
            int data = level->getData(x, y, z);

            auto isLiquidOf = [&](int id, bool& water, bool& lava) {
                water = (id == Tile::water->id || id == Tile::calmWater->id);
                lava  = (id == Tile::lava->id  || id == Tile::calmLava->id);
                return water || lava;
            };

            bool isWater = false, isLava = false;
            int targetX = x, targetY = y, targetZ = z;
            if (!isLiquidOf(t, isWater, isLava)) {
                // Look at the neighbour on the clicked face.
                int nx = x, ny = y, nz = z;
                switch (face) {
                    case 0: --ny; break;
                    case 1: ++ny; break;
                    case 2: --nz; break;
                    case 3: ++nz; break;
                    case 4: --nx; break;
                    case 5: ++nx; break;
                    default: break;
                }
                int nt = level->getTile(nx, ny, nz);
                if (!isLiquidOf(nt, isWater, isLava)) return false;
                targetX = nx; targetY = ny; targetZ = nz;
                t    = nt;
                data = level->getData(nx, ny, nz);
            }

            level->setTile(targetX, targetY, targetZ, 0);
            level->setData(targetX, targetY, targetZ, 0);

            Item* filled = isWater ? Item::bucket_water : Item::bucket_lava;
            if (filled) instance->init(filled->id, 1, 0);
            return true;
        }

        // Filled bucket: place the liquid at the adjacent block face.
        int px = x, py = y, pz = z;
        switch (face) {
            case 0: --py; break;
            case 1: ++py; break;
            case 2: --pz; break;
            case 3: ++pz; break;
            case 4: --px; break;
            case 5: ++px; break;
            default: break;
        }
        int existing = level->getTile(px, py, pz);
        // Only place into empty space (or another liquid we're overwriting).
        if (existing != 0
            && existing != Tile::water->id     && existing != Tile::calmWater->id
            && existing != Tile::lava->id      && existing != Tile::calmLava->id)
        {
            return false;
        }

        // Place the *dynamic* variant (id-1 of the static one) with data=0
        // so it counts as a source block AND immediately starts ticking for
        // flow. Placing the static (calm) tile on dry land just sits there
        // — its conversion to dynamic only kicks in when a neighbour tile
        // changes, which doesn't happen for an isolated water/lava pour.
        int dynamicId = _contents - 1;  // calmWater(9) -> water(8), calmLava(11) -> lava(10)
        level->setTile(px, py, pz, dynamicId);
        level->setData(px, py, pz, 0);
        // Schedule the first flow tick. LiquidTile::getTickDelay() returns
        // 5 (water) or 30 (lava); both fit fine here.
        Tile* placed = Tile::tiles[dynamicId];
        int delay = placed ? placed->getTickDelay() : 5;
        level->addToTickNextTick(px, py, pz, dynamicId, delay);

        // Empty the bucket.
        if (Item::bucket_empty) {
            instance->init(Item::bucket_empty->id, 1, 0);
        }
        return true;
    }

private:
    int _contents;
};

#endif
