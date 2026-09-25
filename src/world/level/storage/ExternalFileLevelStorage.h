#if !defined(DEMO_MODE) && !defined(APPLE_DEMO_PROMOTION)

#ifndef NET_MINECRAFT_WORLD_LEVEL_STORAGE__ExternalFileLevelStorage_H__
#define NET_MINECRAFT_WORLD_LEVEL_STORAGE__ExternalFileLevelStorage_H__

//package net.minecraft.world.level.storage;

#include <vector>
#include <list>
#include <map>

//#include "com/mojang/nbt/CompoundTag.h"
#include "LevelStorage.h"
#include "../chunk/storage/ChunkStorage.h"

class Player;
class Dimension;
class RegionFile;


typedef struct UnsavedLevelChunk
{
	int				pos;
	RakNet::TimeMS	addedToList;
	LevelChunk*		chunk;
} UnsavedLevelChunk;

typedef std::list<UnsavedLevelChunk> UnsavedChunkList;

/*public*/
class ExternalFileLevelStorage:
		public LevelStorage,
		public ChunkStorage
		//public PlayerIO
{
public:
    ExternalFileLevelStorage(const std::string& levelId, const std::string& fullPath);
	virtual ~ExternalFileLevelStorage();

    LevelData* prepareLevel(Level* level);

	//throws LevelConflictException
    void checkSession() {}

    ChunkStorage* createChunkStorage(Dimension* dimension) { return this; }

    void saveLevelData(LevelData& levelData, std::vector<Player*>* players);
    // PlayerIO getPlayerIO() { return this; }
	// CompoundTag loadPlayerDataTag(std::string playerName) { return NULL; }

    void closeAll() {}

	static bool readLevelData(const std::string& directory, LevelData& dest);
	static bool readPlayerData(const std::string& filename, LevelData& dest);
	static bool writeLevelData(const std::string& datFileName, LevelData& dest, const std::vector<Player*>* players);
	static void saveLevelData(const std::string& directory, LevelData& levelData, std::vector<Player*>* players);

    int savePendingUnsavedChunks(int maxCount);

	//
	// ChunkStorage methods
	//
	virtual LevelChunk* load(Level* level, int x, int z);
	void save(Level* level, LevelChunk* levelChunk);
	// @note, loadEntities and saveEntities dont use second parameter
	void loadEntities(Level* level, LevelChunk* levelChunk);
	void saveEntities(Level* level, LevelChunk* levelChunk);
	void saveGame(Level* level);
    void saveAll(Level* level, std::vector<LevelChunk*>& levelChunks);

	virtual void tick();
	virtual void flush() {}

	// Per-dim file routing. activeDimensionId controls which RegionFile and
	// entities file save()/load() target. Switching dims in-game flips this
	// so Nether chunks land in chunks_nether.dat instead of overwriting the
	// overworld's chunks.dat (and vice versa). We flush any pending saves
	// before flipping so chunks queued under the old dim don't get written
	// to the new dim's region file.
	void setActiveDimensionId(int id);
private:
	RegionFile* getRegionFile();         // lazy per-dim chunks file
	std::string entitiesFilePathForDim() const;

	std::string levelId;
	std::string levelPath;
	LevelData* loadedLevelData;
	// regionFiles[dimId] -> chunks.dat for the overworld, chunks_nether.dat
	// for the Nether, etc. Lazily populated; freed in the destructor.
	std::map<int, RegionFile*> regionFiles;
	int activeDimensionId;

	Level* level;
	int tickCount;
	int loadedStorageVersion;
	UnsavedChunkList unsavedChunkList;
	int lastSavedEntitiesTick;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_STORAGE__ExternalFileLevelStorage_H__*/

#endif /*DEMO_MODE*/
