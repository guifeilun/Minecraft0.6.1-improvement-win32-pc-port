#ifndef NET_MINECRAFT_WORLD_ENTITY__Boat_H__
#define NET_MINECRAFT_WORLD_ENTITY__Boat_H__

#include "Entity.h"
#include "EntityTypes.h"

class Player;

class Boat : public Entity
{
public:
    Boat(Level* level);
    Boat(Level* level, double x, double y, double z);

    virtual int getEntityTypeId() const { return EntityTypes::IdBoat; }
    virtual void tick();
    virtual bool interact(Player* player);
    virtual bool isPushable() { return true; }
    virtual bool isPickable() { return !removed; }
    virtual bool hurt(Entity* source, int dmg);
    virtual void positionRider();

    virtual void addAdditonalSaveData(CompoundTag* tag) {}
    virtual void readAdditionalSaveData(CompoundTag* tag) {}

    int timeSinceHit;
    int damageTaken;
    int rockDirection;

private:
    void init();
    double acceleration;
    int boatHealth;
    int healTimer;
};

#endif