#ifndef NET_MINECRAFT_WORLD_LEVEL_DIMENSION__NetherDimension_H__
#define NET_MINECRAFT_WORLD_LEVEL_DIMENSION__NetherDimension_H__

#include "Dimension.h"

// A first-pass Nether dimension for PE 0.6.1:
//   - hasCeiling=true, ultraWarm=true (Java parity)
//   - constant red fog, no day/night cycle
//   - mob spawning gated to a single nether-themed list (just zombie pigmen for
//     now, matching the era — ghasts/magma cubes weren't in PE 0.6.1 either)
//   - chunk generation produces netherrack with bedrock caps, lava ocean at
//     y=31, glowstone clusters on the ceiling, and basic carved caves
class NetherDimension : public Dimension
{
    typedef Dimension super;
public:
    NetherDimension();

    void init();
    bool isValidSpawn(int x, int z);
    // Returns the Y picked by the most recent isValidSpawn() call. Level
    // queries this after the spawn-finder loop succeeds.
    int  getDefaultSpawnY() { return cachedSpawnY > 0 ? cachedSpawnY : 50; }
private:
    int cachedSpawnY;
    bool isNaturalDimension() { return true; }
    float getTimeOfDay(long time, float a);
    float* getSunriseColor(float td, float a);
    Vec3 getFogColor(float td, float a);
    bool mayRespawn();

    ChunkSource* createRandomLevelSource();

protected:
    void updateLightRamp();
};

#endif
