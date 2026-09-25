#include "NetherReactorPattern.h"
#include "Tile.h"
NetherReactorPattern::NetherReactorPattern( ) {
	const int goldId = Tile::goldBlock->id;
	const int stoneId = Tile::stoneBrick->id;
	const int netherCoreId = Tile::netherReactor->id;
	const unsigned int types[3][3][3] =
	{
		// Changing all of these values to be unsigned is needed to get the nether reactor pattern to compile. In the past having them be normal ints was fine but the c++ convention has changed
        // Level 0
		{
            {static_cast<unsigned int>(goldId), static_cast<unsigned int>(stoneId), static_cast<unsigned int>(goldId)},
            {static_cast<unsigned int>(stoneId), static_cast<unsigned int>(stoneId), static_cast<unsigned int>(stoneId)},
            {static_cast<unsigned int>(goldId), static_cast<unsigned int>(stoneId), static_cast<unsigned int>(goldId)}
		},
			// Level 1
		{
            {static_cast<unsigned int>(stoneId), 0, static_cast<unsigned int>(stoneId)},
            {0, static_cast<unsigned int>(netherCoreId), 0},
            {static_cast<unsigned int>(stoneId), 0, static_cast<unsigned int>(stoneId)}
		},
			// Level 2
		{
            {0, static_cast<unsigned int>(stoneId), 0},
            {static_cast<unsigned int>(stoneId), static_cast<unsigned int>(stoneId), static_cast<unsigned int>(stoneId)},
            {0, static_cast<unsigned int>(stoneId), 0}
		}
	};
	for(int setLevel = 0; setLevel <= 2; ++setLevel) {
		for(int setX = 0; setX <= 2; ++setX) {
			for(int setZ = 0; setZ <= 2; ++setZ) {
				setTileAt(setLevel, setX, setZ, types[setLevel][setX][setZ]);
			}
		}
	}
}

void NetherReactorPattern::setTileAt( int level, int x, int z, int tile) {
	pattern[level][x][z] = tile;
}

unsigned int NetherReactorPattern::getTileAt( int level, int x, int z ) {
	return pattern[level][x][z];
}
