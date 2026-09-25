#include "FishingRodItem.h"
#include "../entity/player/Player.h"
#include "../entity/FishingHook.h"
#include "../level/Level.h"

FishingRodItem::FishingRodItem(int id) : Item(id) {
    setMaxStackSize(1);
    setMaxDamage(65);
    handEquipped();
}

ItemInstance* FishingRodItem::use(ItemInstance* instance, Level* level, Player* player) {
    if (player->fishing != NULL) {
        int dmg = ((FishingHook*)player->fishing)->retrieve();
        instance->hurt(dmg);
    } else {
        level->playSound(player, "random.bow", 0.5f,
            0.4f / (random.nextFloat() * 0.4f + 0.8f));
        if (!level->isClientSide) {
            FishingHook* hook = new FishingHook(level, player);
            player->fishing = hook;
            level->addEntity(hook);
        }
    }
    return instance;
}