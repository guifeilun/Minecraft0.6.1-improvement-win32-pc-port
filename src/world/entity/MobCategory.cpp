#ifndef NET_MINECRAFT_WORLD_ENTITY__MobCategory_H__
#define NET_MINECRAFT_WORLD_ENTITY__MobCategory_H__

#include "EntityTypes.h"
#include "MobCategory.h"
#include "../level/material/Material.h"


// Bumped per-level caps to give a livelier world: the old 20-monster cap
// meant that as soon as a few caves were lit you'd never see another mob
// spawn even on the surface at night. 100 is closer to vanilla Java's
// default and lets caves stay populated while still allowing surface
// spawns. Creatures (animals) similarly bumped.
const MobCategory MobCategory::monster(
    MobTypes::BaseEnemy,
    10,
	100,
    false);

	//

const MobCategory MobCategory::creature(
    MobTypes::BaseCreature,
    10,
	30,
    true);

    //

const MobCategory MobCategory::waterCreature(
    MobTypes::BaseWaterCreature,
    5,
	20,
    true);

//
// Init an array with all defined MobCategory'ies
//
const MobCategory* const MobCategory::values[] = {
	&MobCategory::monster,
	&MobCategory::creature,
	&MobCategory::waterCreature
};

/*static*/
void MobCategory::initMobCategories() {
	monster.setMaterial(Material::air);
	creature.setMaterial(Material::air);
	waterCreature.setMaterial(Material::water);
}


const int MobCategory::numValues = sizeof(values) / sizeof(values[0]);

#endif /*NET_MINECRAFT_WORLD_ENTITY__MobCategory_H__*/
