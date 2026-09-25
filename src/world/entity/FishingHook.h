#ifndef NET_MINECRAFT_WORLD_ENTITY__FishingHook_H__
#define NET_MINECRAFT_WORLD_ENTITY__FishingHook_H__

#include "Entity.h"
#include "EntityRendererId.h"

class Player;

class FishingHook : public Entity
{
public:
    FishingHook(Level* level);
    FishingHook(Level* level, Player* owner);

    virtual int getEntityTypeId() const { return 100; }
    virtual void tick();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool shouldRenderAtSqrDistance(float distance);

    void shoot(double xd, double yd, double zd, float pow, float uncertainty);
    int retrieve();

public:
    Player* owner;
    int shakeTime;
    int life;
    int flightTime;
    int nibble;
    Entity* hookedIn;
    bool inGround;
    int xTile, yTile, zTile, lastTile;
};

#endif