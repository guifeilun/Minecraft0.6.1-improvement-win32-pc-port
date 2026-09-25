#include "FishingHookRenderer.h"
#include "EntityRenderDispatcher.h"
#include "../Tesselator.h"
#include "../../../world/entity/FishingHook.h"
#include "../../../world/entity/player/Player.h"
#include "../../../world/item/Item.h"
#include "../../../util/Mth.h"
#include <cmath>

FishingHookRenderer::FishingHookRenderer() {
    shadowRadius = 0.15f;
    shadowStrength = 0.75f;
}

void FishingHookRenderer::render(Entity* entity, float x, float y, float z, float rot, float a) {
    FishingHook* hook = (FishingHook*)entity;

    glPushMatrix2();
    glTranslatef2(x, y, z);
    glScalef2(1 / 2.0f, 1 / 2.0f, 1 / 2.0f);
    bindTexture("particles.png");

    int xi = 1;
    int yi = 2;
    float u0 = (xi * 8 + 0) / 128.0f;
    float u1 = (xi * 8 + 8) / 128.0f;
    float v0 = (yi * 8 + 0) / 128.0f;
    float v1 = (yi * 8 + 8) / 128.0f;

    float r = 1.0f;
    float xo = 0.5f;
    float yo = 0.25f;

    glRotatef2(180 - entityRenderDispatcher->playerRotY, 0, 1, 0);
    glRotatef2(-entityRenderDispatcher->playerRotX, 1, 0, 0);

    Tesselator& t = Tesselator::instance;
    t.begin();
    t.vertexUV(0 - xo, 0 - yo, 0, u0, v1);
    t.vertexUV(r - xo, 0 - yo, 0, u1, v1);
    t.vertexUV(r - xo, 1 - yo, 0, u1, v0);
    t.vertexUV(0 - xo, 1 - yo, 0, u0, v0);
    t.draw();

    glPopMatrix2();

    if (hook->owner != NULL) {
        Player* owner = hook->owner;

        float yaw = -(owner->yRotO + (owner->yRot - owner->yRotO) * a) * Mth::PI / 180.0f;
        float pitch = -(owner->xRotO + (owner->xRot - owner->xRotO) * a) * Mth::PI / 180.0f;

        float vx = -1.0f, vy = 0.03f, vz = 0.8f;

        float cosP = Mth::cos(pitch), sinP = Mth::sin(pitch);
        float cosY = Mth::cos(yaw), sinY = Mth::sin(yaw);

        float vy1 = vy * cosP + vz * sinP;
        float vz1 = vz * cosP - vy * sinP;
        vy = vy1; vz = vz1;

        float vx1 = vx * cosY + vz * sinY;
        float vz2 = vz * cosY - vx * sinY;
        vx = vx1; vz = vz2;

        double xp = owner->xo + (owner->x - owner->xo) * a + vx;
        double yp = owner->yo + (owner->y - owner->yo) * a + vy;
        double zp = owner->zo + (owner->z - owner->zo) * a + vz;

        double xh = hook->xo + (hook->x - hook->xo) * a;
        double yh = hook->yo + (hook->y - hook->yo) * a + 4 / 16.0f;
        double zh = hook->zo + (hook->z - hook->zo) * a;

        double xa = xp - xh;
        double ya = yp - yh;
        double za = zp - zh;

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glColor4f2(0, 0, 0, 1);

        t.begin(GL_LINE_STRIP);
        int steps = 16;
        for (int i = 0; i <= steps; i++) {
            float aa = i / (float)steps;
            t.vertex(
                (float)(x + xa * aa),
                (float)(y + ya * (aa * aa + aa) * 0.5 + 4 / 16.0f),
                (float)(z + za * aa)
            );
        }
        t.draw();

        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
    }
}