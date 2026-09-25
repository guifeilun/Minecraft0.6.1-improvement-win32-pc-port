#include <iostream>
#include "NinecraftApp.h"
#include "AppPlatform.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#if defined(_WIN32)
// <unistd.h> isn't on Windows; sleepMs comes from platform/time.h instead.
#else
#include <unistd.h>
#endif

#include "world/level/LevelSettings.h"
#include "world/level/Level.h"
#include "server/ArgumentsSettings.h"
#include "platform/time.h"
#include "SharedConstants.h"

#define MAIN_CLASS NinecraftApp
static App* g_app = 0;
static int g_exitCode = 0;
void signal_callback_handler(int signum) {
	std::cout << "Signum caught: " << signum << std::endl;
	// Windows has SIGINT (Ctrl+C, 2) and SIGBREAK (Ctrl+Break, 21);
	// POSIX has SIGINT (2) and SIGQUIT (3). Treat any of these as "shut down".
	if (signum == SIGINT
#ifdef _WIN32
		|| signum == SIGBREAK
#else
		|| signum == SIGQUIT
#endif
		) {
		if(g_app != 0) {
			g_app->quit();
		} else {
			exit(g_exitCode);
		}
	}
}

int main(int numArguments, char* pszArgs[]) {
	ArgumentsSettings aSettings(numArguments, pszArgs);
	if(aSettings.getShowHelp()) {
		ArgumentsSettings defaultSettings(0, NULL);
		printf("Minecraft Pockect Edition Server %s\n", Common::getGameVersionString("").c_str());
		printf("-------------------------------------------------------\n");
		printf("Settings are read from server.properties (or the --config path) first;\n");
		printf("command-line flags below override anything in the config file.\n\n");
		printf("--config       Path to the config file. [default: \"server.properties\"]\n");
		printf("--cachepath    Path for temp storage. [default: \"%s\"]\n", defaultSettings.getCachePath().c_str());
		printf("--externalpath Path where the server stores levels. [default: \"%s\"]\n", defaultSettings.getExternalPath().c_str());
		printf("--levelname    The name of the server. [default: \"%s\"]\n", defaultSettings.getLevelName().c_str());
		printf("--leveldir     The directory name of the level. [default: \"%s\"]\n", defaultSettings.getLevelDir().c_str());
		printf("--port         The UDP port to run the server on. [default: %d]\n", defaultSettings.getPort());
		printf("--serverkey    Key used for API calls. [default: \"%s\"]\n", defaultSettings.getServerKey().c_str());
		printf("--gamemode     'survival' or 'creative'. [default: survival]\n");
		printf("--help         Show this message.\n");
		printf("\nExample server.properties:\n");
		printf("  port=19132\n");
		printf("  gamemode=survival\n");
		printf("  levelname=My Server\n");
		printf("  leveldir=world\n");
		printf("-------------------------------------------------------\n");
		return 0;
	}
	printf("Level Name: %s\n", aSettings.getLevelName().c_str());
	printf("Game Mode: %s\n", LevelSettings::gameTypeToString(aSettings.getGameMode()).c_str());
	printf("Port: %d\n", aSettings.getPort());
	AppContext appContext;
	appContext.platform = new AppPlatform();
	App* app = new MAIN_CLASS();
	signal(SIGINT, signal_callback_handler);
#ifdef _WIN32
	signal(SIGBREAK, signal_callback_handler);
#else
	signal(SIGQUIT, signal_callback_handler);
#endif
	g_app = app;
	((MAIN_CLASS*)g_app)->externalStoragePath = aSettings.getExternalPath();
	((MAIN_CLASS*)g_app)->externalCacheStoragePath = aSettings.getCachePath();

	g_app->init(appContext);
	LevelSettings settings(getEpochTimeS(), aSettings.getGameMode());
	float startTime = getTimeS();
	((MAIN_CLASS*)g_app)->selectLevel(aSettings.getLevelDir(), aSettings.getLevelName(),  settings);
	((MAIN_CLASS*)g_app)->hostMultiplayer(aSettings.getPort());

	std::cout << "Level has been generated in " << getTimeS() - startTime << std::endl;
	((MAIN_CLASS*)g_app)->level->saveLevelData();
	std::cout << "Level has been saved!" << std::endl;

	while(!app->wantToQuit()) {
		app->update();
		//pthread_yield();
		sleepMs(20);
	}
	((MAIN_CLASS*)g_app)->level->saveLevelData();
	delete app;
	appContext.platform->finish();
	delete appContext.platform;

	std::cout << "Quit correctly" << std::endl;
	return g_exitCode;
}