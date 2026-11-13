#include "SharedConstants.h"

namespace Common {

std::string getGameVersionString(const std::string& versionSuffix /* = "" */)
{
	std::string result = std::string("v0.6.1") + versionSuffix;
	result += " alpha";
	return result;
}

};
