#ifndef NET_MINECRAFT_WORLD_LEVEL__ArgumentsSettings_H__
#define NET_MINECRAFT_WORLD_LEVEL__ArgumentsSettings_H__
#include <string>
class ArgumentsSettings {
public:
	// Reads server.properties (if present) first, then applies command-line
	// arguments on top so CLI overrides the config file.
	ArgumentsSettings(int numArguments, char** arguments);
	std::string getExternalPath();
	std::string getLevelName();
	std::string getServerKey();
	std::string getCachePath();
	std::string getLevelDir();
	std::string getConfigPath();
	bool getShowHelp();
	int getPort();
	// Returns GameType::Survival (0) or GameType::Creative (1).
	int getGameMode();
private:
	// Parse a single key=value entry and apply it. Returns true if recognized.
	bool applySetting(const std::string& key, const std::string& value);
	// Load all key=value pairs from a config file. Lines starting with '#' or
	// ';' are comments; blank lines are skipped. Silently ignored if missing.
	void loadConfigFile(const std::string& path);

	std::string configPath;
	std::string cachePath;
	std::string externalPath;
	std::string levelName;
	std::string levelDir;
	std::string serverKey;
	bool showHelp;
	int port;
	int gameMode;
};

#endif
