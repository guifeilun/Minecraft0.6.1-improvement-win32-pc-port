#ifndef NET_MINECRAFT_WORLD_ITEM__SpawnEggItem_H__
#define NET_MINECRAFT_WORLD_ITEM__SpawnEggItem_H__

#include "Item.h"
#include <string>

class ItemInstance;
class Level;
class Player;
class Mob;

class SpawnEggItem : public Item {
    typedef Item super;
public:
    static const std::string EGG_ENTITY_KEYS[];
    static const int EGG_TYPE_IDS[];
    static const int EGG_TYPE_COUNT;

    SpawnEggItem(int id);

    int getIcon(int itemAuxValue);
    std::string getDescriptionId(const ItemInstance* itemInstance) const;
    bool useOn(ItemInstance* itemInstance, Player* player, Level* level,
               int x, int y, int z, int face, float clickX, float clickY, float clickZ);
};

#endif /*NET_MINECRAFT_WORLD_ITEM__SpawnEggItem_H__*/