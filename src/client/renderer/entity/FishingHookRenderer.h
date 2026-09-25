#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__FishingHookRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__FishingHookRenderer_H__

#include "EntityRenderer.h"

class FishingHookRenderer : public EntityRenderer
{
public:
    FishingHookRenderer();
    virtual void render(Entity* entity, float x, float y, float z, float rot, float a);
};

#endif