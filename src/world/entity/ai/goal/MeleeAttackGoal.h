#ifndef NET_MINECRAFT_WORLD_ENTITY_AI_GOAL__MeleeAttackGoal_H__
#define NET_MINECRAFT_WORLD_ENTITY_AI_GOAL__MeleeAttackGoal_H__

//package net.minecraft.world.entity.ai.goal;

#include "Goal.h"

#include "../control/Control.h"
#include "../../monster/Monster.h"
#include "../../../level/Level.h"
#include "../../../level/pathfinder/Path.h"
#include "../Sensing.h"

class MeleeAttackGoal: public Goal
{
public:
    MeleeAttackGoal(Monster* mob, float speed, bool trackTarget, int attackType = 0)
        :       mob(mob),
                level(mob->level),
                speed(speed),
                trackTarget(trackTarget),
                attackTime(0),
                attackType(attackType)
        {
                setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
    }
        ~MeleeAttackGoal() {
    }

    bool canUse() {
        Mob* bestTarget = mob->getTarget();
        if (bestTarget == NULL) return false;
        if (attackType != 0 && !mob->isPlayer()) return false;
        target = bestTarget;
        // PathNavigation owns the path; don't keep a copy.
        return mob->getNavigation()->moveTo(target, speed);
    }

    bool canContinueToUse() {
        Mob* bestTarget = mob->getTarget();
        if (bestTarget == NULL) return false;
        if (attackType != 0 && !mob->isPlayer()) return false;
        target = bestTarget;
        if (!trackTarget) return !mob->getNavigation()->isDone();
        return true;
    }

    void start() {
        // moveTo already called in canUse()
    }

    void stop() {
        target = NULL;
        mob->getNavigation()->stop();
    }

    void tick() {
        if (trackTarget || mob->sensing->canSee(target)) {
            mob->getNavigation()->moveTo(target, speed);
        }

        attackTime = Mth::Max(attackTime - 1, 0);

        float meleeRadiusSqr = (mob->bbWidth * 2) * (mob->bbWidth * 2);
        if (mob->distanceToSqr(target->x, target->bb.y0, target->z) > meleeRadiusSqr) return;
        if (attackTime > 0) return;
        attackTime = 20;
        mob->doHurtTarget(target);
    }

private:
    Level* level;
    Monster* mob;
    Mob* target;
    int attackTime;
    float speed;
    int attackType;
    bool trackTarget;
};

#endif /*NET_MINECRAFT_WORLD_ENTITY_AI_GOAL__MeleeAttackGoal_H__*/