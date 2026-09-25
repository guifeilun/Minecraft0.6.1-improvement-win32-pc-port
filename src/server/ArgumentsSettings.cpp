#include "ArgumentsSettings.h"
#include "../world/level/LevelSettings.h"  // GameType constants
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fstream>
#include <sstream>

namespace {
std::string trim(const std::string& s) {
	size_t start = 0;
	while (start < s.size() && (unsigned char)s[start] <= ' ') ++start;
	size_t end = s.size();
	while (end > start && (unsigned char)s[end - 1] <= ' ') --end;
	return s.substr(start, end - start);
}

std::string toLower(const std::string& s) {
	std::string out = s;
	for (size_t i = 0; i < out.size(); ++i)
		out[i] = (char)tolower((unsigned char)out[i]);
	return out;
}
}

ArgumentsSettings::ArgumentsSettings(int numArguments, char** arguments)
:	configPath("server.properties"),
	cachePath("."),
	externalPath("."),
	levelName("level"),
	levelDir("level"),
	serverKey(""),
	showHelp(false),
	port(19132),
	gameMode(GameType::Survival)
{
	// First pass over the CLI: see if a --config was specified so we know which
	// file to load. (Defaults to "server.properties" in the working directory.)
	for (int a = 0; a < numArguments; ++a) {
		if (strcmp(arguments[a], "--config") == 0 && a + 1 < numArguments) {
			configPath = arguments[a + 1];
		}
	}

	// Load defaults from the config file (if it exists).
	loadConfigFile(configPath);

	// Then apply CLI overrides — these win over the config file.
	for (int a = 0; a < numArguments; ++a) {
		if (strcmp(arguments[a], "--help") == 0) {
			showHelp = true;
		} else if (strcmp(arguments[a], "--config") == 0 && a + 1 < numArguments) {
			a++;  // already consumed above
		} else if (strcmp(arguments[a], "--externalpath") == 0 && a + 1 < numArguments) {
			externalPath = arguments[++a];
		} else if (strcmp(arguments[a], "--levelname") == 0 && a + 1 < numArguments) {
			levelName = arguments[++a];
		} else if (strcmp(arguments[a], "--leveldir") == 0 && a + 1 < numArguments) {
			levelDir = arguments[++a];
		} else if (strcmp(arguments[a], "--port") == 0 && a + 1 < numArguments) {
			port = atoi(arguments[++a]);
		} else if (strcmp(arguments[a], "--serverkey") == 0 && a + 1 < numArguments) {
			serverKey = arguments[++a];
		} else if (strcmp(arguments[a], "--cachepath") == 0 && a + 1 < numArguments) {
			cachePath = arguments[++a];
		} else if (strcmp(arguments[a], "--gamemode") == 0 && a + 1 < numArguments) {
			applySetting("gamemode", arguments[++a]);
		}
	}
}

bool ArgumentsSettings::applySetting(const std::string& rawKey, const std::string& rawValue) {
	std::string key = toLower(trim(rawKey));
	std::string value = trim(rawValue);

	if (key == "port") { port = atoi(value.c_str()); return true; }
	if (key == "externalpath") { externalPath = value; return true; }
	if (key == "levelname") { levelName = value; return true; }
	if (key == "leveldir") { levelDir = value; return true; }
	if (key == "cachepath") { cachePath = value; return true; }
	if (key == "serverkey") { serverKey = value; return true; }
	if (key == "gamemode") {
		std::string v = toLower(value);
		if (v == "survival" || v == "0" || v == "s") gameMode = GameType::Survival;
		else if (v == "creative" || v == "1" || v == "c") gameMode = GameType::Creative;
		else fprintf(stderr, "Unknown gamemode '%s' (expected survival|creative). Ignoring.\n", value.c_str());
		return true;
	}
	return false;
}

void ArgumentsSettings::loadConfigFile(const std::string& path) {
	std::ifstream in(path.c_str());
	if (!in.is_open()) return;  // file doesn't exist — fine, just use defaults

	std::string line;
	int lineNo = 0;
	while (std::getline(in, line)) {
		++lineNo;
		std::string trimmed = trim(line);
		if (trimmed.empty()) continue;
		if (trimmed[0] == '#' || trimmed[0] == ';') continue;  // comment

		size_t eq = trimmed.find('=');
		if (eq == std::string::npos) {
			fprintf(stderr, "%s:%d: skipping malformed line (no '='): %s\n",
				path.c_str(), lineNo, trimmed.c_str());
			continue;
		}
		std::string key = trimmed.substr(0, eq);
		std::string value = trimmed.substr(eq + 1);

		if (!applySetting(key, value)) {
			fprintf(stderr, "%s:%d: unknown setting '%s', ignoring.\n",
				path.c_str(), lineNo, trim(key).c_str());
		}
	}

	fprintf(stdout, "Loaded server settings from %s\n", path.c_str());
}

std::string ArgumentsSettings::getExternalPath() { return externalPath; }
std::string ArgumentsSettings::getLevelName()    { return levelName; }
std::string ArgumentsSettings::getServerKey()    { return serverKey; }
std::string ArgumentsSettings::getCachePath()    { return cachePath; }
std::string ArgumentsSettings::getLevelDir()     { return levelDir; }
std::string ArgumentsSettings::getConfigPath()   { return configPath; }
bool        ArgumentsSettings::getShowHelp()     { return showHelp; }
int         ArgumentsSettings::getPort()         { return port; }
int         ArgumentsSettings::getGameMode()     { return gameMode; }
