#ifndef NET_MINECRAFT_WORLD_LEVEL__LevelSettings_H__
#define NET_MINECRAFT_WORLD_LEVEL__LevelSettings_H__

//package net.minecraft.world.level;

namespace GameType {
	const int Undefined = -1;
	const int Survival = 0;
	const int Creative = 1;

	const int Default = Creative;
}

namespace WorldType {
	const int Old      = 0; // finite 256x256 world
	const int Infinite = 1; // infinite procedural world
}

class LevelSettings
{
public:
    LevelSettings(long seed, int gameType, int worldType = WorldType::Old, bool allowCheats = false)
    :   seed(seed),
        gameType(gameType),
        worldType(worldType),
        allowCheats(allowCheats)
    {
    }
	static LevelSettings None() {
		return LevelSettings(-1, -1, WorldType::Old, false);
	}

    long getSeed() const {
        return seed;
    }

    int getGameType() const {
        return gameType;
    }

    int getWorldType() const {
        return worldType;
    }

    bool getAllowCheats() const {
        return allowCheats;
    }

	//
	// Those two should actually not be here
	// @todo: Move out when we add LevelSettings.cpp :p
	//
	static int validateGameType(int gameType) {
        switch (gameType) {
		case GameType::Creative:
		case GameType::Survival:
            return gameType;
        }
        return GameType::Default;
    }

	static std::string gameTypeToString(int gameType) {
		if (gameType == GameType::Survival) return "Survival";
		if (gameType == GameType::Creative) return "Creative";
		return "Undefined";
	}

private:
    const long seed;
    const int gameType;
    const int worldType;
    const bool allowCheats;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL__LevelSettings_H__*/