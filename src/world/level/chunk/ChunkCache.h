#ifndef NET_MINECRAFT_WORLD_LEVEL_CHUNK__ChunkCache_H__
#define NET_MINECRAFT_WORLD_LEVEL_CHUNK__ChunkCache_H__

//package net.minecraft.world.level.chunk;

#include "ChunkSource.h"
#include "storage/ChunkStorage.h"
#include "EmptyLevelChunk.h"
#include "../Level.h"
#include "../LevelConstants.h"
#include <unordered_map>
#include <cstdint>

class ChunkCache: public ChunkSource {
    //static const int CHUNK_CACHE_WIDTH = CHUNK_CACHE_WIDTH; // WAS 32;
    static const int MAX_SAVES = 2;
public:
    ChunkCache(Level* level_, ChunkStorage* storage_, ChunkSource* source_)
        :       xLast(-999999999),
                zLast(-999999999),
                last(NULL),
                level(level_),
                storage(storage_),
                source(source_)
        {
                isChunkCache = true;
                emptyChunk = new EmptyLevelChunk(level_, NULL, 0, 0);
        }

        ~ChunkCache() {
                delete source;
                delete emptyChunk;
                for (auto& kv : chunks) {
                        if (kv.second && kv.second != emptyChunk) {
                                kv.second->deleteBlockData();
                                delete kv.second;
                        }
                }
        }

        static int64_t chunkKey(int x, int z) {
                return ((int64_t)(uint32_t)x << 32) | (uint32_t)z;
        }

    bool fits(int x, int z) {
        if (level->isInfinite()) return true;
        return (x >= 0 && z >= 0 && x < CHUNK_CACHE_WIDTH && z < CHUNK_CACHE_WIDTH);
    }

    bool hasChunk(int x, int z) {
        // with a fixed world size, chunks outside the fitting area are always available (emptyChunks)
        if (!fits(x, z)) return true;

        if (x == xLast && z == zLast && last != NULL) {
            return true;
        }
        int64_t key = chunkKey(x, z);
        auto it = chunks.find(key);
        return it != chunks.end() && (it->second == emptyChunk || it->second->isAt(x, z));
    }

    LevelChunk* create(int x, int z) {
        return getChunk(x, z);
    }

    LevelChunk* getChunk(int x, int z) {
                if (x == xLast && z == zLast && last != NULL) {
            return last;
        }
                if (!fits(x, z)) return emptyChunk;

        int64_t key = chunkKey(x, z);
        if (!hasChunk(x, z)) {
            LevelChunk* newChunk = load(x, z);
            bool updateLights = false;
            if (newChunk == NULL) {
                if (source == NULL) {
                    newChunk = emptyChunk;
                } else {
                    newChunk = source->getChunk(x, z);
                }
            } else {
                updateLights = true;
            }
            chunks[key] = newChunk;
            newChunk->lightLava();

            if (updateLights)
            {
                for (int cx = 0; cx < 16; cx++)
                {
                    for (int cz = 0; cz < 16; cz++)
                    {
                        int height = level->getHeightmap(cx + x * 16, cz + z * 16);
                        for (int cy = height; cy >= 0; cy--)
                        {
                            level->updateLight(LightLayer::Sky, cx + x * 16, cy, cz + z * 16, cx + x * 16, cy, cz + z * 16);
                            level->updateLight(LightLayer::Block, cx + x * 16 - 1, cy, cz + z * 16 - 1, cx + x * 16 + 1, cy, cz + z * 16 + 1);
                        }
                    }
                }
            }

            LevelChunk* stored = chunks[key];
            if (stored != NULL) {
                stored->load();
            }

            // Tell the renderer that this chunk's blocks exist now, so it
            // rebuilds the boundary faces with already-rendered neighbours.
            level->setTilesDirty(x * 16, 0, z * 16, x * 16 + 15, 127, z * 16 + 15);

            if (!chunks[key]->terrainPopulated && hasChunk(x + 1, z + 1) && hasChunk(x, z + 1) && hasChunk(x + 1, z)) postProcess(this, x, z);
            if (hasChunk(x - 1, z) && !getChunk(x - 1, z)->terrainPopulated && hasChunk(x - 1, z + 1) && hasChunk(x, z + 1) && hasChunk(x - 1, z)) postProcess(this, x - 1, z);
            if (hasChunk(x, z - 1) && !getChunk(x, z - 1)->terrainPopulated && hasChunk(x + 1, z - 1) && hasChunk(x, z - 1) && hasChunk(x + 1, z)) postProcess(this, x, z - 1);
            if (hasChunk(x - 1, z - 1) && !getChunk(x - 1, z - 1)->terrainPopulated && hasChunk(x - 1, z - 1) && hasChunk(x, z - 1) && hasChunk(x - 1, z)) postProcess(this, x - 1, z - 1);
        }
        xLast = x;
        zLast = z;
        last = chunks[key];

        return chunks[key];
    }

        Biome::MobList getMobsAt(const MobCategory& mobCategory, int x, int y, int z) {
                return source->getMobsAt(mobCategory, x, y, z);
        }

    void postProcess(ChunkSource* parent, int x, int z) {
                if (!fits(x, z)) return;
        LevelChunk* chunk = getChunk(x, z);
        if (!chunk->terrainPopulated) {
            chunk->terrainPopulated = true;
            if (source != NULL) {
                source->postProcess(parent, x, z);
                                chunk->clearUpdateMap();
            }
        }
    }

    bool tick() {
        if (storage != NULL) storage->tick();
        return source->tick();
    }

    bool shouldSave() {
        return true;
    }

    std::string gatherStats() {
        std::stringstream ss;
        ss << "ChunkCache: " << chunks.size();
        return ss.str();
    }

    void saveAll(bool onlyUnsaved) {
        if (storage != NULL) {
            std::vector<LevelChunk*> chunksToSave;
            for (auto& kv : chunks) {
                LevelChunk* chunk = kv.second;
                if (chunk && chunk != emptyChunk)
                    if (!onlyUnsaved || chunk->shouldSave(false))
                        chunksToSave.push_back(chunk);
            }
            storage->saveAll(level, chunksToSave);
        }
    }
private:
    LevelChunk* load(int x, int z) {
        if (storage == NULL) return NULL;
        LevelChunk* levelChunk = storage->load(level, x, z);
        if (levelChunk != NULL) {
            levelChunk->lastSaveTime = level->getTime();
        }
        return levelChunk;
    }

    void saveEntities(LevelChunk* levelChunk) {
        if (storage == NULL) return;
        storage->saveEntities(level, levelChunk);
    }

    void save(LevelChunk* levelChunk) {
        if (storage == NULL) return;
        levelChunk->lastSaveTime = level->getTime();
        storage->save(level, levelChunk);
    }

public:
        int xLast;
    int zLast;
private:
    LevelChunk* emptyChunk;
    ChunkSource* source;
    ChunkStorage* storage;
    std::unordered_map<int64_t, LevelChunk*> chunks;
    Level* level;

    LevelChunk* last;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_CHUNK__ChunkCache_H__*/