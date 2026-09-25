#include "NetherLevelSource.h"
#include "../Level.h"
#include "../ChunkPos.h"
#include "../tile/Tile.h"
#include "../chunk/LevelChunk.h"
#include "../LevelConstants.h"
#include "../../entity/MobCategory.h"
#include "../../entity/EntityTypes.h"
#include "../biome/Biome.h"
#include "feature/OreFeature.h"
#include "../../../util/Random.h"

namespace {
// Cheap deterministic 32-bit hash mixing world coordinates and seed.
unsigned int hashAt(int x, int y, int z, long seed) {
    unsigned int h = (unsigned int)seed;
    h = h * 0x9E3779B1u + (unsigned int)x;
    h = h * 0x9E3779B1u + (unsigned int)y;
    h = h * 0x9E3779B1u + (unsigned int)z;
    h ^= h >> 16;
    h *= 0x85EBCA6Bu;
    h ^= h >> 13;
    return h;
}

inline int blockIndex(int x, int y, int z) {
    return (x << 11) | (z << 7) | y;
}
}

NetherLevelSource::NetherLevelSource(Level* level_, long seed_)
:   level(level_),
    seed(seed_),
    rng(seed_),
    random(seed_),
    terrainNoise(&rng, 4),
    detailNoise(&rng, 2)
{
}

NetherLevelSource::~NetherLevelSource()
{
}

bool NetherLevelSource::hasChunk(int x, int z) {
    return chunkMap.find(ChunkPos::hashCode(x, z)) != chunkMap.end();
}
LevelChunk* NetherLevelSource::create(int x, int z) { return getChunk(x, z); }

LevelChunk* NetherLevelSource::getChunk(int chunkX, int chunkZ) {
    int hashedPos = ChunkPos::hashCode(chunkX, chunkZ);
    std::map<int, LevelChunk*>::iterator it = chunkMap.find(hashedPos);
    if (it != chunkMap.end()) return it->second;

    random.setSeed((long)(chunkX * 341872712l + chunkZ * 132899541l));

    LOGI("[NetherLS] GEN chunk (%d, %d) tiles: nr=%p bd=%p lv=%p lg=%p\n",
        chunkX, chunkZ,
        Tile::netherrack, Tile::unbreakable, Tile::lava, Tile::lightGem);

    const int LAVA_TOP = 30;
    const int FLOOR_TOP = 33;
    const int CAVERN_TOP = 68;
    const int DENSE_TOP = 118;
    const int CEILING = 127;

    unsigned char* blocks = new unsigned char[LevelChunk::ChunkBlockCount];
    const unsigned char NETHERRACK = (unsigned char)Tile::netherrack->id;
    const unsigned char BEDROCK    = (unsigned char)Tile::unbreakable->id;
    const unsigned char LAVA       = (unsigned char)Tile::lava->id;
    const unsigned char AIR        = 0;
    const unsigned char GLOWSTONE  = (unsigned char)Tile::lightGem->id;

    for (int x = 0; x < 16; ++x)
    for (int z = 0; z < 16; ++z) {
        int wx = chunkX * 16 + x;
        int wz = chunkZ * 16 + z;

        for (int y = 0; y <= CEILING; ++y) {
            int idx = blockIndex(x, y, z);
            unsigned char block = NETHERRACK;

            if (y == 0) {
                block = BEDROCK;
            } else if (y == CEILING) {
                block = BEDROCK;
            } else if (y == CEILING - 1) {
                block = ((hashAt(wx, y, wz, seed) % 10) < 5) ? BEDROCK : NETHERRACK;
            } else if (y == 1) {
                block = ((hashAt(wx, y, wz, seed) % 10) < 6) ? BEDROCK : NETHERRACK;
            } else {
                float base = terrainNoise.getValue(wx * 0.05f, y * 0.10f, wz * 0.05f);
                float detail = detailNoise.getValue(wx * 0.20f, y * 0.20f, wz * 0.20f);
                float density = base + detail * 0.20f;

                float threshold;
                if (y <= LAVA_TOP) {
                    threshold = 0.85f;
                } else if (y <= FLOOR_TOP) {
                    threshold = -0.15f;
                } else if (y <= CAVERN_TOP) {
                    threshold = 0.55f;
                } else if (y <= DENSE_TOP) {
                    float t = (float)(y - CAVERN_TOP) / (float)(DENSE_TOP - CAVERN_TOP);
                    threshold = 0.55f - 0.55f * t;
                } else {
                    threshold = -0.25f;
                }

                bool solid = density > threshold;
                if (solid) {
                    block = NETHERRACK;
                } else if (y <= LAVA_TOP) {
                    block = LAVA;
                } else {
                    block = AIR;
                }
            }

            blocks[idx] = block;
        }
    }

    // Glowstone clusters
    const int CELL = 24;
    const int chunkBaseCellX = (chunkX * 16) / CELL - 1;
    const int chunkBaseCellZ = (chunkZ * 16) / CELL - 1;

    for (int cdx = 0; cdx <= 2; ++cdx)
    for (int cdz = 0; cdz <= 2; ++cdz) {
        int cellX = chunkBaseCellX + cdx;
        int cellZ = chunkBaseCellZ + cdz;

        unsigned int h = hashAt(cellX, 0, cellZ, seed ^ 0xC0FFEEu);
        if ((h % 3) != 0) continue;

        int ax = cellX * CELL + (int)((h >> 4)  & 0x1F) % CELL;
        int az = cellZ * CELL + (int)((h >> 9)  & 0x1F) % CELL;
        int ay = CEILING - 2 - (int)((h >> 14) & 0x3);

        float radius = 4.0f + (float)((h >> 17) & 0x3);

        int minX = ax - (int)radius - chunkX * 16;
        int maxX = ax + (int)radius - chunkX * 16;
        int minZ = az - (int)radius - chunkZ * 16;
        int maxZ = az + (int)radius - chunkZ * 16;
        if (maxX < 0 || minX >= 16) continue;
        if (maxZ < 0 || minZ >= 16) continue;

        if (minX < 0)  minX = 0;
        if (maxX > 15) maxX = 15;
        if (minZ < 0)  minZ = 0;
        if (maxZ > 15) maxZ = 15;

        const float Y_STRETCH = 0.55f;

        int yLo = ay - (int)(radius / Y_STRETCH);
        int yHi = ay + 1;
        if (yLo < 1) yLo = 1;
        if (yHi > CEILING - 1) yHi = CEILING - 1;

        for (int x = minX; x <= maxX; ++x)
        for (int z = minZ; z <= maxZ; ++z)
        for (int y = yLo; y <= yHi; ++y) {
            int wx = chunkX * 16 + x;
            int wz = chunkZ * 16 + z;
            float fx = (float)(wx - ax);
            float fz = (float)(wz - az);
            float fy = (float)(y  - ay) * Y_STRETCH;
            float d2 = fx * fx + fy * fy + fz * fz;
            float r2 = radius * radius;
            if (d2 > r2) continue;
            unsigned int j = hashAt(wx, y, wz, seed ^ 0xF00DBABEu);
            float jitter = ((j & 0xFF) / 255.0f) * 0.8f;
            if (d2 + jitter * jitter * radius > r2) continue;

            int idx = blockIndex(x, y, z);
            if (blocks[idx] == NETHERRACK) {
                blocks[idx] = GLOWSTONE;
            }
        }
    }

    LOGI("[NetherLS::getChunk] terrain pass done, creating LevelChunk\n");
    LevelChunk* chunk = new LevelChunk(level, blocks, chunkX, chunkZ);
    chunkMap.insert(std::make_pair(hashedPos, chunk));
    chunk->recalcHeightmap();
    LOGI("[NetherLS::getChunk] recalcHeightmap done, returning\n");
    return chunk;
}

void NetherLevelSource::postProcess(ChunkSource* parent, int xt, int zt) {
    int xo = xt * 16;
    int zo = zt * 16;

    random.setSeed(level->getSeed());
    int xScale = random.nextInt() / 2 * 2 + 1;
    int zScale = random.nextInt() / 2 * 2 + 1;
    random.setSeed(((xt * xScale) + (zt * zScale)) ^ level->getSeed());

    for (int i = 0; i < 16; i++) {
        int x = xo + random.nextInt(16);
        int y = 10 + random.nextInt(108);
        int z = zo + random.nextInt(16);
        OreFeature feature(Tile::quartzOre->id, 14);
        feature.place(level, &random, x, y, z);
    }
}

bool NetherLevelSource::tick() { return false; }
bool NetherLevelSource::shouldSave() { return true; }

Biome::MobList NetherLevelSource::getMobsAt(const MobCategory& mobCategory, int x, int y, int z)
{
    Biome::MobList list;
    if (&mobCategory == &MobCategory::monster) {
        list.insert(list.end(), Biome::MobSpawnerData(MobTypes::PigZombie, 100, 4, 4));
    }
    return list;
}

std::string NetherLevelSource::gatherStats() {
    return "NetherLevelSource";
}