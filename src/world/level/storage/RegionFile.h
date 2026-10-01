#ifndef NET_MINECRAFT_WORLD_LEVEL_STORAGE__RegionFile_H__
#define NET_MINECRAFT_WORLD_LEVEL_STORAGE__RegionFile_H__

#include <map>
#include <string>
#include <vector>
#include "../../../raknet/BitStream.h"

typedef std::map<int, bool> FreeSectorMap;

class RegionFile
{
public:
	RegionFile(const std::string& basePath);
	// Variant that overrides the default "chunks.dat" filename — used so
	// the Nether can store its chunks alongside the overworld's in the same
	// level folder ("chunks_nether.dat") without overwriting them.
	RegionFile(const std::string& basePath, const std::string& fileName);
	// Variant that takes a complete file path. Used by infinite worlds where
	// each 32x32-chunk region goes to its own "chunks.r.<rx>.<rz>.dat".
	RegionFile(const std::string& fullFilePath, bool isFullPath);
	virtual ~RegionFile();

	bool open();
	bool readChunk(int x, int z, RakNet::BitStream** destChunkData);
	bool writeChunk(int x, int z, RakNet::BitStream& chunkData);

	// Read the entire file into memory so subsequent readChunk() calls are
	// pure memory accesses. Call this once after open().
	bool preloadToMemory();
	void freeMemoryCache();

private:
	bool write(int sector, RakNet::BitStream& chunkData);
	void close();

	FILE* file;
	std::string	filename;
	int* offsets;
	int* emptyChunk;
	FreeSectorMap sectorFree;

	std::vector<unsigned char> _fileCache;
};


#endif /*NET_MINECRAFT_WORLD_LEVEL_STORAGE__RegionFile_H__*/