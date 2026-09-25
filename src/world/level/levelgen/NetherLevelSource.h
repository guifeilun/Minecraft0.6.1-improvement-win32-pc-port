#ifndef NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__NetherLevelSource_H__
#define NET_MINECRAFT_WORLD_LEVEL_LEVELGEN__NetherLevelSource_H__

#include <map>
#include "../chunk/ChunkSource.h"
#include "../biome/Biome.h"
#include "../../../util/Random.h"
#include "synth/PerlinNoise.h"

class Level;
class LevelChunk;

class NetherLevelSource : public ChunkSource
{
public:
    NetherLevelSource(Level* level, long seed);
    ~NetherLevelSource();

    bool hasChunk(int x, int z);
    LevelChunk* getChunk(int x, int z);
    LevelChunk* create(int x, int z);
    void postProcess(ChunkSource* parent, int x, int z);
    bool tick();
    bool shouldSave();
    Biome::MobList getMobsAt(const MobCategory& mobCategory, int x, int y, int z);
    std::string gatherStats();

private:
    Level* level;
    long seed;
    Random rng;
    Random random;
    PerlinNoise terrainNoise;
    PerlinNoise detailNoise;
    std::map<int, LevelChunk*> chunkMap;
};

#endif