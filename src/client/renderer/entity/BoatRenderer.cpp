#include "BoatRenderer.h"
#include "../../../world/entity/Boat.h"
#include "../Tesselator.h"
#include "../../../util/Mth.h"

BoatRenderer::BoatRenderer() {
    shadowRadius = 0.5f;
}

void BoatRenderer::render(Entity* entity, float x, float y, float z, float rot, float a) {
    Boat* boat = (Boat*)entity;

    glPushMatrix2();
    glTranslatef2(x, y - 0.3f, z);
    glRotatef2(180.0f - rot, 0, 1, 0);

    float hurt = boat->timeSinceHit - a;
    float dmg = boat->damageTaken - a;
    if (dmg < 0.0f) dmg = 0.0f;
    if (hurt > 0.0f) {
        glRotatef2(Mth::sin(hurt) * hurt * dmg * 0.1f * (float)boat->rockDirection, 0.0f, 0.0f, 1.0f);
    }

    bindTexture("terrain.png");

    Tesselator& t = Tesselator::instance;
    t.begin();

    float u0 = 4.0f / 16.0f;
    float u1 = 5.0f / 16.0f;
    float v0 = 0.0f / 16.0f;
    float v1 = 1.0f / 16.0f;

    float hw = 0.7f;
    float hl = 1.0f;
    float h = 0.6f;
    
    float iw = hw - 0.15f;
    float il = hl - 0.15f;
    float ih = 0.2f;

    // --- Outer Walls ---
    // Bottom
    t.vertexUV(-hw, 0, -hl, u0, v0); t.vertexUV( hw, 0, -hl, u1, v0); t.vertexUV( hw, 0,  hl, u1, v1); t.vertexUV(-hw, 0,  hl, u0, v1);
    // Front (-Z)
    t.vertexUV(-hw, 0, -hl, u0, v0); t.vertexUV(-hw, h, -hl, u0, v1); t.vertexUV( hw, h, -hl, u1, v1); t.vertexUV( hw, 0, -hl, u1, v0);
    // Back (+Z)
    t.vertexUV( hw, 0,  hl, u0, v0); t.vertexUV( hw, h,  hl, u0, v1); t.vertexUV(-hw, h,  hl, u1, v1); t.vertexUV(-hw, 0,  hl, u1, v0);
    // Left (-X)
    t.vertexUV(-hw, 0,  hl, u0, v0); t.vertexUV(-hw, h,  hl, u0, v1); t.vertexUV(-hw, h, -hl, u1, v1); t.vertexUV(-hw, 0, -hl, u1, v0);
    // Right (+X)
    t.vertexUV( hw, 0, -hl, u0, v0); t.vertexUV( hw, h, -hl, u0, v1); t.vertexUV( hw, h,  hl, u1, v1); t.vertexUV( hw, 0,  hl, u1, v0);

    // --- Inner Walls ---
    // Inner Bottom
    t.vertexUV(-iw, ih,  il, u0, v0); t.vertexUV( iw, ih,  il, u1, v0); t.vertexUV( iw, ih, -il, u1, v1); t.vertexUV(-iw, ih, -il, u0, v1);
    // Inner Front
    t.vertexUV(-iw, ih, -il, u0, v0); t.vertexUV( iw, ih, -il, u1, v0); t.vertexUV( iw, h, -il, u1, v1); t.vertexUV(-iw, h, -il, u0, v1);
    // Inner Back
    t.vertexUV( iw, ih,  il, u0, v0); t.vertexUV(-iw, ih,  il, u1, v0); t.vertexUV(-iw, h,  il, u1, v1); t.vertexUV( iw, h,  il, u0, v1);
    // Inner Left
    t.vertexUV(-iw, ih,  il, u0, v0); t.vertexUV(-iw, ih, -il, u1, v0); t.vertexUV(-iw, h, -il, u1, v1); t.vertexUV(-iw, h,  il, u0, v1);
    // Inner Right
    t.vertexUV( iw, ih, -il, u0, v0); t.vertexUV( iw, ih,  il, u1, v0); t.vertexUV( iw, h,  il, u1, v1); t.vertexUV( iw, h, -il, u0, v1);

    // --- Top Edges (Connecting inner and outer walls) ---
    // Front Edge
   t.vertexUV(-iw, h, -il, u0, v1); t.vertexUV( iw, h, -il, u1, v1); t.vertexUV( hw, h, -hl, u1, v0); t.vertexUV(-hw, h, -hl, u0, v0);
    // Back Edge
    t.vertexUV( iw, h,  il, u0, v1); t.vertexUV(-iw, h,  il, u1, v1); t.vertexUV(-hw, h,  hl, u1, v0); t.vertexUV( hw, h,  hl, u0, v0);
    // Left Edge
    t.vertexUV(-hw, h, -hl, u0, v0); t.vertexUV(-hw, h,  hl, u1, v0); t.vertexUV(-iw, h,  il, u1, v1); t.vertexUV(-iw, h, -il, u0, v1);
    // Right Edge
    t.vertexUV( hw, h,  hl, u0, v0); t.vertexUV( hw, h, -hl, u1, v0); t.vertexUV( iw, h, -il, u1, v1); t.vertexUV( iw, h,  il, u0, v1);

    t.draw();
    glPopMatrix2();
}