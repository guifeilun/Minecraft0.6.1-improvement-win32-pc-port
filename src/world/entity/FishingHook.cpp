#include "FishingHook.h"
#include "player/Player.h"
#include "item/ItemEntity.h"
#include "../level/Level.h"
#include "../level/material/Material.h"
#include "../item/Item.h"
#include "../item/ItemInstance.h"
#include "../../util/Mth.h"
#include <cmath>

FishingHook::FishingHook(Level* level) : Entity(level) {
    xTile = -1; yTile = -1; zTile = -1;
    lastTile = 0;
    inGround = false;
    shakeTime = 0;
    flightTime = 0;
    nibble = 0;
    hookedIn = NULL;
    owner = NULL;
    life = 0;
    setSize(0.25f, 0.25f);
    entityRendererId = ER_FISHING_HOOK_RENDERER;
}

FishingHook::FishingHook(Level* level, Player* owner) : Entity(level) {
    xTile = -1; yTile = -1; zTile = -1;
    lastTile = 0;
    inGround = false;
    shakeTime = 0;
    flightTime = 0;
    nibble = 0;
    hookedIn = NULL;
    this->owner = owner;
    life = 0;
    setSize(0.25f, 0.25f);
    entityRendererId = ER_FISHING_HOOK_RENDERER;

    moveTo(owner->x, owner->y + 1.62f - owner->heightOffset, owner->z,
           owner->yRot, owner->xRot);
    x -= Mth::cos(yRot / 180 * Mth::PI) * 0.16f;
    y -= 0.1f;
    z -= Mth::sin(yRot / 180 * Mth::PI) * 0.16f;
    setPos(x, y, z);
    heightOffset = 0;

    float speed = 0.4f;
    xd = -Mth::sin(yRot / 180 * Mth::PI) * Mth::cos(xRot / 180 * Mth::PI) * speed;
    zd =  Mth::cos(yRot / 180 * Mth::PI) * Mth::cos(xRot / 180 * Mth::PI) * speed;
    yd = -Mth::sin(xRot / 180 * Mth::PI) * speed;
    shoot(xd, yd, zd, 1.5f, 1);
}

void FishingHook::shoot(double xd, double yd, double zd, float pow, float uncertainty) {
    float dist = (float)sqrt(xd * xd + yd * yd + zd * zd);
    xd /= dist; yd /= dist; zd /= dist;

    xd += sharedRandom.nextGaussian() * 0.0075f * uncertainty;
    yd += sharedRandom.nextGaussian() * 0.0075f * uncertainty;
    zd += sharedRandom.nextGaussian() * 0.0075f * uncertainty;

    xd *= pow; yd *= pow; zd *= pow;

    this->xd = xd;
    this->yd = yd;
    this->zd = zd;

    double sd = sqrt(xd * xd + zd * zd);
    yRotO = yRot = (float)(atan2(xd, zd) * 180 / Mth::PI);
    xRotO = xRot = (float)(atan2(yd, sd) * 180 / Mth::PI);
    life = 0;
}

void FishingHook::tick() {
    Entity::tick();

    if (!level->isClientSide) {
        ItemInstance* selected = owner->getSelectedItem();
        if (owner->removed || !owner->isAlive()
            || selected == NULL || selected->getItem() != Item::fishingRod
            || distanceToSqr(owner) > 32 * 32) {
            remove();
            owner->fishing = NULL;
            return;
        }
    }

    if (shakeTime > 0) shakeTime--;

    if (inGround) {
        int tile = level->getTile(xTile, yTile, zTile);
        if (tile != lastTile) {
            life++;
            if (life == 20 * 60) remove();
            return;
        } else {
            inGround = false;
            xd *= sharedRandom.nextFloat() * 0.2f;
            yd *= sharedRandom.nextFloat() * 0.2f;
            zd *= sharedRandom.nextFloat() * 0.2f;
            life = 0;
            flightTime = 0;
        }
    } else {
        flightTime++;
    }

    Vec3 from(x, y, z);
    Vec3 to(x + xd, y + yd, z + zd);
    HitResult res = level->clip(from, to);

    if (res.isHit()) {
        to = Vec3(res.pos.x, res.pos.y, res.pos.z);
    }

    Entity* hitEntity = NULL;
    double nearest = 0;
    AABB searchBox = bb.expand(xd, yd, zd).grow(1, 1, 1);
    EntityList& objects = level->getEntities(this, searchBox);
    for (size_t i = 0; i < objects.size(); i++) {
        Entity* e = objects[i];
        if (!e->isPickable() || (e == owner && flightTime < 5)) continue;
        float rr = 0.3f;
        AABB ebb = e->bb.grow(rr, rr, rr);
        HitResult p = ebb.clip(from, to);
        if (p.isHit()) {
            double dd = from.distanceTo(p.pos);
            if (dd < nearest || nearest == 0) {
                hitEntity = e;
                nearest = dd;
            }
        }
    }

    if (hitEntity != NULL) {
        hookedIn = hitEntity;
    } else if (res.isHit()) {
        inGround = true;
        xTile = res.x;
        yTile = res.y;
        zTile = res.z;
        lastTile = level->getTile(xTile, yTile, zTile);
    }

    if (inGround) return;

    move(xd, yd, zd);

    double sd = sqrt(xd * xd + zd * zd);
    yRot = (float)(atan2(xd, zd) * 180 / Mth::PI);
    xRot = (float)(atan2(yd, sd) * 180 / Mth::PI);

    while (xRot - xRotO < -180) xRotO -= 360;
    while (xRot - xRotO >= 180) xRotO += 360;
    while (yRot - yRotO < -180) yRotO -= 360;
    while (yRot - yRotO >= 180) yRotO += 360;

    xRot = xRotO + (xRot - xRotO) * 0.2f;
    yRot = yRotO + (yRot - yRotO) * 0.2f;

    float inertia = 0.92f;
    if (onGround || horizontalCollision) inertia = 0.5f;

    int steps = 5;
    double waterPercentage = 0;
    for (int i = 0; i < steps; i++) {
        double y0 = bb.y0 + (bb.y1 - bb.y0) * i / steps;
        double y1 = bb.y0 + (bb.y1 - bb.y0) * (i + 1) / steps;
        AABB bb2(bb.x0, y0, bb.z0, bb.x1, y1, bb.z1);
        if (level->containsLiquid(bb2, Material::water)) {
            waterPercentage += 1.0 / steps;
        }
    }

    if (waterPercentage > 0) {
        if (nibble > 0) {
            nibble--;
        } else {
            if (sharedRandom.nextInt(500) == 0) {
                nibble = sharedRandom.nextInt(30) + 10;
                yd -= 0.2f;
                level->playSound(this, "random.splash", 0.25f,
                    1 + (sharedRandom.nextFloat() - sharedRandom.nextFloat()) * 0.4f);
            }
        }
    }

    if (nibble > 0) {
        yd -= sharedRandom.nextFloat() * sharedRandom.nextFloat() * sharedRandom.nextFloat() * 0.2;
    }

    double bob = waterPercentage * 2 - 1;
    yd += 0.04f * bob;
    if (waterPercentage > 0) {
        inertia *= 0.9;
        yd *= 0.8;
    }

    xd *= inertia;
    yd *= inertia;
    zd *= inertia;

    setPos(x, y, z);
}

int FishingHook::retrieve() {
    if (level->isClientSide) return 0;

    int dmg = 0;
    if (hookedIn != NULL) {
        double xa = owner->x - x;
        double ya = owner->y - y;
        double za = owner->z - z;
        double dist = sqrt(xa * xa + ya * ya + za * za);
        double speed = 0.1;
        hookedIn->xd += xa * speed;
        hookedIn->yd += ya * speed + sqrt(dist) * 0.08;
        hookedIn->zd += za * speed;
        dmg = 3;
    } else if (nibble > 0) {
        ItemEntity* ie = new ItemEntity(level, x, y, z, ItemInstance(Item::fish_raw));
        double xa = owner->x - x;
        double ya = owner->y - y;
        double za = owner->z - z;
        double dist = sqrt(xa * xa + ya * ya + za * za);
        double speed = 0.1;
        ie->xd = xa * speed;
        ie->yd = ya * speed + sqrt(dist) * 0.08;
        ie->zd = za * speed;
        level->addEntity(ie);
        dmg = 1;
    }
    if (inGround) dmg = 2;

    remove();
    owner->fishing = NULL;
    return dmg;
}

void FishingHook::addAdditonalSaveData(CompoundTag* tag) {}
void FishingHook::readAdditionalSaveData(CompoundTag* tag) {}

bool FishingHook::shouldRenderAtSqrDistance(float distance) {
    return true;
}