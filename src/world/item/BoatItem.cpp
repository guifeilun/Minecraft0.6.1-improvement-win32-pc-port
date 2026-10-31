#include "BoatItem.h"
#include "../entity/Boat.h"
#include "../level/Level.h"
#include "../entity/player/Player.h"
#include "../phys/HitResult.h"
#include "../../util/Mth.h"

BoatItem::BoatItem(int id) : Item(id) {
    setMaxStackSize(1);
}

ItemInstance* BoatItem::use(ItemInstance* itemInstance, Level* level, Player* player) {
    if (level->isClientSide) return itemInstance;

    double eyeY = player->y + 1.62f - player->heightOffset;
    Vec3 from(player->x, eyeY, player->z);
    Vec3 to(
        player->x - Mth::sin(player->yRot * Mth::DEGRAD) * 5.0f,
        eyeY - Mth::sin(player->xRot * Mth::DEGRAD) * 5.0f,
        player->z + Mth::cos(player->yRot * Mth::DEGRAD) * 5.0f
    );
    HitResult hr = level->clip(from, to, true);

    if (hr.isHit()) {
        Boat* boat = new Boat(level, hr.x + 0.5f, hr.y + 1.5f, hr.z + 0.5f);
        boat->yRot = player->yRot;
        level->addEntity(boat);

        if (!player->abilities.instabuild) {
            --itemInstance->count;
        }
    }
    return itemInstance;
}