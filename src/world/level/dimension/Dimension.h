#ifndef NET_MINECRAFT_WORLD_LEVEL_DIMENSION__Dimension_H__
#define NET_MINECRAFT_WORLD_LEVEL_DIMENSION__Dimension_H__

//package net.minecraft.world.level.dimension;

#include "../../phys/Vec3.h"

class Level;
class BiomeSource;
class ChunkSource;

class Dimension
{
public:
	static const int NORMAL = 0;
	static const int NETHER = 1;
	static const int NORMAL_DAYCYCLE = 10;

	Dimension();
	virtual ~Dimension();
    virtual void init(Level* level);

	//@fix @port The caller is responsible for this ChunkSource, I presume
    virtual ChunkSource* createRandomLevelSource();

    virtual bool isValidSpawn(int x, int z);
	virtual bool isNaturalDimension() {
		return false;
	}

	// Default spawn Y for new worlds in this dimension. Overworld uses 64;
	// the Nether keeps players inside the main cavern band.
	virtual int  getDefaultSpawnY() { return 64; }

    virtual float getTimeOfDay(long time, float a);
    virtual float* getSunriseColor(float td, float a);
    virtual Vec3 getFogColor(float td, float a);

    virtual bool mayRespawn();

	// @fix @port Caller is responsible (+ move this to a "factory method" outside?)
	// @NOTE: RIGHT NOW, Level deletes the dimension.
    static Dimension* getNew(int id);

public:
	// Public so the brightness slider can recompute the active dim's ramp
	// immediately when the user moves it (see Minecraft::optionUpdated).
	virtual void updateLightRamp();
protected:
	virtual void init();

public:
	Level* level;
	BiomeSource* biomeSource;
	bool foggy;
	bool ultraWarm;
	bool hasCeiling;
	float brightnessRamp[16];//Level::MAX_BRIGHTNESS + 1];
	int id;
	// Java-style gamma. Shared across dimensions so the options-slider only
	// needs to write one value and any active dim's ramp is recomputed
	// against it. Range 0..1: 0 = Moody (no boost), 1 = Bright! (max boost,
	// minimum scene brightness lifted to ~0.5).
	static float s_brightness;
	std::string getDimension();
	// shredder added
	int FogType; // lets us choose between what fog we want ig
protected:
	long fogColor; //= 0x80daff;//0x406fe5;//0xc0d8ff;
	float sunriseCol[4];
};

class LevelData;
class DimensionFactory
{
public:
	static Dimension* createDefaultDimension(LevelData* data);
	// Debug toggle: when true, createDefaultDimension returns a NetherDimension
	// regardless of the level data. Set this before triggering world creation.
	static bool s_forceNether;
};

#endif /*NET_MINECRAFT_WORLD_LEVEL_DIMENSION__Dimension_H__*/
