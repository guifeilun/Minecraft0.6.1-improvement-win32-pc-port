#include "NetherDimension.h"
#include "../Level.h"
#include "../biome/BiomeSource.h"
#include "../chunk/ChunkSource.h"
#include "../tile/Tile.h"
#include "../../../util/Mth.h"
#include "../levelgen/NetherLevelSource.h"

NetherDimension::NetherDimension()
:   cachedSpawnY(-1)
{
    foggy = true;
    ultraWarm = true;
    hasCeiling = true;
    fogColor = 0x330808;  // deep red, like Java's nether
    id = NETHER;
}

void NetherDimension::init()
{
    // Reuse the overworld BiomeSource so existing surface/cave logic doesn't
    // crash — the nether chunk source generates its own surface anyway and
    // ignores biome output.
    biomeSource = new BiomeSource(level);
}

bool NetherDimension::isValidSpawn(int x, int z)
{
    LOGI("[NetherDim::isValidSpawn] start at (%d,?,%d)\n", x, z);
    if (!level) { LOGI("[NetherDim::isValidSpawn] level is NULL\n"); return false; }
    // Scan downward through the open cavern band looking for the first solid
    // (non-lava, non-air) block with at least two air blocks above it.
    for (int y = 64; y >= 33; --y) {
        int t = level->getTile(x, y, z);
        if (t == 0) continue;
        if (t == Tile::lava->id || t == Tile::calmLava->id) {
            LOGI("[NetherDim::isValidSpawn] lava at y=%d, bail\n", y);
            return false;
        }
        if (t < 0 || t >= 256 || !Tile::tiles[t]) {
            LOGI("[NetherDim::isValidSpawn] unknown tile id %d at y=%d\n", t, y);
            continue;
        }
        if (!Tile::tiles[t]->isSolidRender()) continue;
        int t1 = level->getTile(x, y + 1, z);
        int t2 = level->getTile(x, y + 2, z);
        if (t1 != 0 || t2 != 0) {
            LOGI("[NetherDim::isValidSpawn] no headroom at y=%d\n", y);
            return false;
        }
        cachedSpawnY = y + 1;
        LOGI("[NetherDim::isValidSpawn] found safe Y=%d\n", cachedSpawnY);
        return true;
    }
    LOGI("[NetherDim::isValidSpawn] no safe spot found\n");
    return false;
}

float NetherDimension::getTimeOfDay(long time, float a)
{
    return 0.5f;  // perpetual "noon" — the sky is the ceiling here anyway
}

float* NetherDimension::getSunriseColor(float td, float a)
{
    return NULL;  // no sun = no sunrise
}

Vec3 NetherDimension::getFogColor(float td, float a)
{
    // Static red fog regardless of "time of day".
    long c = fogColor;
    float r = ((c >> 16) & 0xff) / 255.0f;
    float g = ((c >> 8)  & 0xff) / 255.0f;
    float b = ((c)       & 0xff) / 255.0f;
    return Vec3(r, g, b);
}

bool NetherDimension::mayRespawn()
{
    // Java: dying in the Nether sends you to the Overworld spawn. We don't
    // have inter-dimension respawn wired yet, so for v1 just say yes — player
    // respawns at the Nether spawn point until that wiring exists.
    return true;
}

void NetherDimension::updateLightRamp()
{
    // The Nether has a noticeable ambient glow (Java sets light at every level
    // to a minimum of about 7/15ths). Approximate that with a flatter ramp.
    float ambient = 0.10f;
    float gamma = Dimension::s_brightness;
    for (int i = 0; i <= 15; ++i) {
        float v = 1 - i / 15.0f;
        float bright = ((1 - v) / (v * 3 + 1)) * (1 - ambient) + ambient;
        // Bias upward so the Nether is never pitch-black.
        if (bright < 0.18f) bright = 0.18f;
        float base = bright * 3;
        if (gamma > 0.5f) {
            float t = (gamma - 0.5f) * 2.0f;
            float lifted = base > 0.5f ? base : 0.5f;
            base = base + (lifted - base) * t;
        } else if (gamma < 0.5f) {
            float t = (0.5f - gamma) * 2.0f;
            base *= (1.0f - t * 0.6f);
        }
        brightnessRamp[i] = base;
    }
}

ChunkSource* NetherDimension::createRandomLevelSource()
{
    return new NetherLevelSource(level, level->getSeed());
}
