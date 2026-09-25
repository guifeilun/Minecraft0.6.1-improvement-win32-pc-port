#include "Dimension.h"
#include "NormalDayCycleDimension.h"
#include "NetherDimension.h"

//#include "../levelgen/SimpleLevelSource.h"
#include "../levelgen/RandomLevelSource.h"
#include "../Level.h"
#include "../biome/BiomeSource.h"
#include "../chunk/ChunkSource.h"
#include "../tile/Tile.h"
#include "../../../util/Mth.h"


Dimension::Dimension()
:	foggy(false),
	ultraWarm(false),
	hasCeiling(false),
	biomeSource(NULL),
	id(0),
	fogColor(0x80daff)
{
}

Dimension::~Dimension()
{
	delete biomeSource;
}

void Dimension::init( Level* level )
{
	this->level = level;
	init();
	updateLightRamp();
}

void Dimension::init()
{
	biomeSource = new BiomeSource(level);
}

/*virtual*/
bool Dimension::isValidSpawn(int x, int z) {
    int topTile = level->getTopTile(x, z);

	if (topTile == Tile::invisible_bedrock->id)
		return false;

    //if (topTile != Tile::sand->id) return false;
	if (!Tile::tiles[topTile]->isSolidRender()) return false;

    return true;
}

float Dimension::getTimeOfDay(long time, float a) {
	return 1;
}

ChunkSource* Dimension::createRandomLevelSource() {
	return new RandomLevelSource(
		level,
		level->getSeed(),
		level->getLevelData()->getGeneratorVersion(),
		!level->isClientSide && level->getLevelData()->getSpawnMobs());
	//return new PerformanceTestChunkSource(level);
}


float Dimension::s_brightness = 0.5f;

void Dimension::updateLightRamp()
{
	float ambient = 0.02f;
	float gamma = Dimension::s_brightness;
	for (int i = 0; i <= 15; ++i) {
		// Pow curve: nearly black at low light, full white at light 15. The
		// old ((1-v)/(v*3+1)) curve plus the +ambient*3 floor produced a
		// brightnessRamp[4] of ~0.21 — that's bright enough at midnight
		// (effective sky-light 4) to make the surface look like dusk.
		// Power 2.5 gives ramp[4] ≈ 0.06, properly dark.
		float t = i / 15.0f;
		float base = powf(t, 2.5f) * (1.0f - ambient) + ambient;
		// Centred gamma slider — 0.5 is the historic look. Above 0.5 lifts
		// dark levels toward ~0.5 (Java's "Bright!"); below 0.5 darkens.
		if (gamma > 0.5f) {
			float k = (gamma - 0.5f) * 2.0f;
			float lifted = base > 0.5f ? base : 0.5f;
			base = base + (lifted - base) * k;
		} else if (gamma < 0.5f) {
			float k = (0.5f - gamma) * 2.0f;
			base *= (1.0f - k * 0.6f);
		}
		brightnessRamp[i] = base;
	}
}

float* Dimension::getSunriseColor( float td, float a )
{
	float span = 0.4f;
	float tt = Mth::cos(td * Mth::PI * 2) - 0.0f;
	float mid = -0.0f;
	if (tt >= mid - span && tt <= mid + span) {
		float aa = ((tt - mid) / span) * 0.5f + 0.5f;
		float mix = 1 - (((1 - Mth::sin(aa * Mth::PI))) * 0.99f);
		mix = mix * mix;
		sunriseCol[0] = (aa * 0.3f + 0.7f);
		sunriseCol[1] = (aa * aa * 0.7f + 0.2f);
		sunriseCol[2] = (aa * aa * 0.0f + 0.2f);
		sunriseCol[3] = mix;
		return sunriseCol;
	}
	return NULL;
}

Vec3 Dimension::getFogColor( float td, float a )
{
	if (FogType == 1)
	{
		fogColor = 0xc0d8ff; // 1 returns java beta styled fog color.
	} 
	else if (FogType == 2)
	{
		fogColor = 0x406fe5; // 2 returns some type of unused fog color IDK what this one was used for possibly early pe??
	}
	else // otherwise as default we return the mcpe fog color
	{
		fogColor = 0x80daff;
	}
	float br = Mth::cos(td * Mth::PI * 2) * 2 + 0.5f;
	if (br < 0.0f) br = 0.0f;
	if (br > 1.0f) br = 1.0f;

	float r = ((fogColor >> 16) & 0xff) / 255.0f;
	float g = ((fogColor >> 8) & 0xff) / 255.0f;
	float b = ((fogColor) & 0xff) / 255.0f;
	r *= br * 0.94f + 0.06f;
	g *= br * 0.94f + 0.06f;
	b *= br * 0.91f + 0.09f;
	return Vec3(r, g, b);
	
	 //
}

bool Dimension::mayRespawn()
{
	return true;
}

Dimension* Dimension::getNew( int id )
{
	if (id == NORMAL) return new Dimension();
	if (id == NETHER) return new NetherDimension();
	if (id == NORMAL_DAYCYCLE) return new NormalDayCycleDimension();
	return NULL;
}

std::string Dimension::getDimension(){
	int currentID = this->id;
	if (currentID == Dimension::NORMAL) return "Overworld";
	if (currentID == Dimension::NORMAL_DAYCYCLE) return "Overworld";
	if (currentID == Dimension::NETHER) return "Nether";
	return "Unknown";
}

//
// DimensionFactory
//
#include "../storage/LevelData.h"

bool DimensionFactory::s_forceNether = false;

Dimension* DimensionFactory::createDefaultDimension(LevelData* data)
{
	if (s_forceNether) {
		return Dimension::getNew(Dimension::NETHER);
	}

	// Honour a saved dim id when it indicates a non-overworld choice. We
	// only special-case NETHER here because the overworld variants (NORMAL
	// vs NORMAL_DAYCYCLE) are selected by game type and the LevelData
	// default of 0 collides with NORMAL — there's no way to distinguish
	// "freshly created, never saved" from "deliberately saved as NORMAL".
	if (data->getDimension() == Dimension::NETHER) {
		return Dimension::getNew(Dimension::NETHER);
	}

	int dimensionId = Dimension::NORMAL;
	switch(data->getGameType()) {
	case GameType::Survival: dimensionId = Dimension::NORMAL_DAYCYCLE;
		break;
	case GameType::Creative:
	default:
		dimensionId = Dimension::NORMAL;
		break;
	}
	return Dimension::getNew(dimensionId);
}
