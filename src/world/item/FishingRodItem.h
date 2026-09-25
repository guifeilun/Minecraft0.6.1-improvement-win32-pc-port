#ifndef NET_MINECRAFT_WORLD_ITEM__FishingRodItem_H__
#define NET_MINECRAFT_WORLD_ITEM__FishingRodItem_H__

#include "Item.h"

class Player;
class Level;

class FishingRodItem : public Item
{
public:
    FishingRodItem(int id);
    virtual ItemInstance* use(ItemInstance* instance, Level* level, Player* player);
};

#endif