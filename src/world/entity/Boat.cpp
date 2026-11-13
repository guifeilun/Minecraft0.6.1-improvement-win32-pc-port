#include "Boat.h"
#include "player/Player.h"
#include "item/ItemEntity.h"
#include "../item/Item.h"
#include "../level/Level.h"
#include "../level/material/Material.h"
#include "../../util/Mth.h"
#include <cmath>

static double wrapDegrees(double angle) {
    while (angle >= 180.0) angle -= 360.0;
    while (angle < -180.0) angle += 360.0;
    return angle;
}

Boat::Boat(Level* level) : Entity(level) {
    init();
}

Boat::Boat(Level* level, double x, double y, double z) : Entity(level) {
    init();
    setPos(x, y, z);
    xd = yd = zd = 0;
}

void Boat::init() {
    entityRendererId = ER_BOAT_RENDERER;
    setSize(1.4f, 0.6f);
    heightOffset = bbHeight / 2.0f;
    timeSinceHit = 0;
    damageTaken = 0;
    rockDirection = 1;
    acceleration = 0.07;
    blocksBuilding = true;
    boatHealth = 4;
    healTimer = 0;
}

bool Boat::interact(Player* player) {
    if (rider == player) return true;
    if (rider != NULL) return true;
    if (!level->isClientSide) {
        player->startRiding(this);
    }
    return true;
}

bool Boat::hurt(Entity* source, int dmg) {
    if (removed || level->isClientSide) return true;

    boatHealth -= dmg;
    timeSinceHit = 10;
    damageTaken += dmg * 5;
    rockDirection = -rockDirection;
    healTimer = 0;

    if (boatHealth <= 0) {
        remove();
        if (Item::boat != NULL) {
            spawnAtLocation(Item::boat->id, 1);
        }
    }
    return true;
}

void Boat::positionRider() {
    if (rider == NULL) return;
    rider->setPos(x, y + bbHeight * 1.5f, z);
    rider->yRot = yRot;
}

void Boat::tick() {
    Entity::tick();
    if (timeSinceHit > 0) timeSinceHit--;
    if (damageTaken > 0 && (tickCount % 2 == 0)) damageTaken--;

    int steps = 5;
    double waterPercentage = 0;
    for (int i = 0; i < steps; i++) {
        double y0 = bb.y0 + (bb.y1 - bb.y0) * i / steps - 2 / 16.0f;
        double y1 = bb.y0 + (bb.y1 - bb.y0) * (i + 1) / steps - 2 / 16.0f;
        AABB bb2(bb.x0, y0, bb.z0, bb.x1, y1, bb.z1);
        if (level->containsLiquid(bb2, Material::water)) {
            waterPercentage += 1.0 / steps;
        }
    }

    if (waterPercentage < 1) {
        double bob = waterPercentage * 2 - 1;
        yd += 0.04 * bob;
    } else {
        if (yd < 0) yd /= 2;
        yd += 0.007;
    }

    if (rider != NULL && rider->isMob()) {
        Mob* mobRider = (Mob*)rider;

        if (mobRider->getMoveForward() > 0) {
            double riderXd = -Mth::sin(yRot * Mth::DEGRAD);
            double riderZd =  Mth::cos(yRot * Mth::DEGRAD);
            xd += riderXd * mobRider->getMoveForward() * acceleration * 0.05;
            zd += riderZd * mobRider->getMoveForward() * acceleration * 0.05;
        }

        if (mobRider->getMoveStrafe() != 0) {
            double speed = sqrt(xd * xd + zd * zd);
            float turnSpeed = 0.05f + (float)(speed * 10.0f);
            yRot += mobRider->getMoveStrafe() * turnSpeed;
        }
    }

    double curSpeed = sqrt(xd * xd + zd * zd);
    if (curSpeed > 0.35) {
        double ratio = 0.35 / curSpeed;
        xd *= ratio;
        zd *= ratio;
    }

    if (onGround) {
        xd *= 0.5f; yd *= 0.5f; zd *= 0.5f;
    }
    move(xd, yd, zd);

    xd *= 0.99f; yd *= 0.95f; zd *= 0.99f;

    if (rider != NULL) {
        positionRider();
    }

    if (boatHealth > 0 && boatHealth < 4) {
        if (timeSinceHit > 0) {
            healTimer = 0;
        } else {
            healTimer++;
            if (healTimer >= 20) {
                boatHealth++;
                healTimer = 0;
            }
        }
    }
}
