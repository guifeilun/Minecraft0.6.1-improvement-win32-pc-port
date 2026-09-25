#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__CakeTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__CakeTile_H__

#include "Tile.h"
#include "../material/Material.h"
#include "../Level.h"
#include "../../entity/player/Player.h"

// Cake. AuxValue stores how many slices have been eaten (0..6). Right-click
// removes one slice and heals the player by 2 HP. After the 6th slice the
// tile is removed entirely. Drops nothing — the only way to recover the
// cake is to bake another one.
class CakeTile: public Tile
{
    typedef Tile super;
public:
    CakeTile(int id)
    :   super(id, 9 + 7 * 16 /* cake top */, Material::vegetable)
    {
    }

    int getTexture(int face) {
        if (face == 1) return 9  + 7 * 16; // top
        if (face == 0) return 12 + 7 * 16; // bottom
        // No data context: render the uncut side.
        return 10 + 7 * 16;
    }

    int getTexture(LevelSource* level, int x, int y, int z, int face) {
        if (face == 1) return 9  + 7 * 16;
        if (face == 0) return 12 + 7 * 16;
        // Show the cut side facing west once any slice has been eaten.
        int bites = level->getData(x, y, z);
        if (bites > 0 && face == 4 /* WEST */) return 11 + 7 * 16;
        return 10 + 7 * 16;
    }

    AABB* getAABB(Level* level, int x, int y, int z) {
        int bites = level->getData(x, y, z);
        // Vanilla: 1 px inset on top/bottom/N/S; 1 + 2*bites px on west.
        float pix = 1.0f / 16.0f;
        float westInset = pix + (float)bites * 2.0f * pix;
        tmpBB.set(
            (float)x + westInset, (float)y,
            (float)z + pix,
            (float)x + 1.0f - pix, (float)y + 0.5f,
            (float)z + 1.0f - pix);
        return &tmpBB;
    }

    void updateShape(LevelSource* level, int x, int y, int z) {
        int bites = level->getData(x, y, z);
        float pix = 1.0f / 16.0f;
        float westInset = pix + (float)bites * 2.0f * pix;
        setShape(westInset, 0.0f, pix, 1.0f - pix, 0.5f, 1.0f - pix);
    }

    // Resets the shape to the uncut full-cake AABB before the inventory icon
    // is rendered. Without this the tile renderer would draw the cake with
    // whatever AABB was last set on the shared Tile instance (often garbage
    // left over from another cake elsewhere in the world).
    void updateDefaultShape() {
        float pix = 1.0f / 16.0f;
        setShape(pix, 0.0f, pix, 1.0f - pix, 0.5f, 1.0f - pix);
    }

    bool isCubeShaped()  { return false; }
    bool isSolidRender() { return false; }
    int  getResource(int data, Random* random) { return 0; }
    int  getResourceCount(Random* random)      { return 0; }

    bool mayPlace(Level* level, int x, int y, int z) {
        // Need a solid block below — same as torches/saplings.
        if (!super::mayPlace(level, x, y, z)) return false;
        int below = level->getTile(x, y - 1, z);
        if (below == 0) return false;
        Tile* t = Tile::tiles[below];
        return t && t->isSolidRender();
    }

    bool use(Level* level, int x, int y, int z, Player* player) {
        if (!player) return false;
        // Some versions gate this on hunger; we don't have a hunger bar wired,
        // so just always heal a small amount per slice. Caps at MAX_HEALTH.
        if (player->health < Player::MAX_HEALTH) {
            player->heal(2);
        }
        int bites = level->getData(x, y, z);
        if (bites < 6) {
            level->setData(x, y, z, bites + 1);
            // Refresh AABB / rendering after the slice is taken.
            level->setTilesDirty(x, y, z, x, y, z);
        } else {
            // Last slice consumed — remove the cake.
            level->setTile(x, y, z, 0);
        }
        return true;
    }
};

#endif
