#ifndef NET_MINECRAFT_WORLD_ITEM__BoatItem_H__
#define NET_MINECRAFT_WORLD_ITEM__BoatItem_H__

#include "Item.h"

class Player;
class Level;

class BoatItem : public Item {
public:
    BoatItem(int id);
    virtual ItemInstance* use(ItemInstance* itemInstance, Level* level, Player* player);
};

#endif