#ifndef NINECRAFTAPP_H__
#define NINECRAFTAPP_H__

#include "world/Pos.h"
#include "App.h"
#include "client/Minecraft.h"
#include "world/level/storage/MemoryLevelStorage.h"
#include <string>


class Level;
class LocalPlayer;
class ExternalFileLevelStorageSource;

class NinecraftApp: public Minecraft
{
public:
    NinecraftApp();
	~NinecraftApp();

	void init();
	void teardown();

	void update();

	virtual bool handleBack(bool isDown);

protected:
	void onGraphicsReset();

private:
	void initGLStates();
    void restartServer();

	//void writeDemoFile();
	//bool hasPlayedDemo();

	void testCreationAndDestruction();
	void testJoiningAndDestruction();

	bool _verbose;
	int _frames;
};

#endif//NINECRAFTAPP_H__
