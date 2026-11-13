#include "SpawnEggItem.h"
#include "ItemInstance.h"
#include "../entity/Mob.h"
#include "../entity/MobFactory.h"
#include "../entity/EntityTypes.h"
#include "../entity/player/Player.h"
#include "../level/Level.h"
#include "../../util/Mth.h"

const std::string SpawnEggItem::EGG_ENTITY_KEYS[] = {
    "Chicken", "Cow", "Pig", "Sheep",
    "Zombie", "Creeper", "Skeleton", "Spider", "PigZombie"
};

const int SpawnEggItem::EGG_TYPE_IDS[] = {
    MobTypes::Chicken, MobTypes::Cow, MobTypes::Pig, MobTypes::Sheep,
    MobTypes::Zombie, MobTypes::Creeper, MobTypes::Skeleton, MobTypes::Spider, MobTypes::PigZombie
};

const int SpawnEggItem::EGG_TYPE_COUNT = 9;

SpawnEggItem::SpawnEggItem(int id) : super(id) {
    setStackedByData(true);
    setMaxDamage(0);
}

int SpawnEggItem::getIcon(int itemAuxValue) {
    return icon; 
}

std::string SpawnEggItem::getDescriptionId(const ItemInstance* itemInstance) const {
    int aux = Mth::clamp(itemInstance->getAuxValue(), 0, EGG_TYPE_COUNT - 1);
    return "entity." + EGG_ENTITY_KEYS[aux];
}

bool SpawnEggItem::useOn(ItemInstance* itemInstance, Player* player, Level* level,
                         int x, int y, int z, int face, float clickX, float clickY, float clickZ) {
    if (level->isClientSide) return true; 

    int aux = Mth::clamp(itemInstance->getAuxValue(), 0, EGG_TYPE_COUNT - 1);
    int typeId = EGG_TYPE_IDS[aux];

    Mob* mob = MobFactory::CreateMob(typeId, level);
    if (!mob) return false;

    mob->moveTo(x + 0.5f, y + 1.0f, z + 0.5f, 0.0f, 0.0f);
    level->addEntity(mob);

    if (!player->abilities.invulnerable) {
        itemInstance->count--;
    }
    return true;
}