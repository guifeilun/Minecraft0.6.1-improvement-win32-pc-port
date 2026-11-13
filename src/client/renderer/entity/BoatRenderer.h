#ifndef NET_MINECRAFT_CLIENT_RENDERER_ENTITY__BoatRenderer_H__
#define NET_MINECRAFT_CLIENT_RENDERER_ENTITY__BoatRenderer_H__

#include "EntityRenderer.h"

class BoatRenderer : public EntityRenderer
{
public:
    BoatRenderer();
    virtual void render(Entity* entity, float x, float y, float z, float rot, float a);
};

#endif