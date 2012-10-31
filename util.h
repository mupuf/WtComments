#ifndef UTIL_H
#define UTIL_H
	#include <string>

	std::string &strReplace(std::string & subj, const std::string &old, const std::string &neu);
	unsigned int countOccurencies(const std::string &str, const std::string &substr);
	std::string getExeDirectory();
#endif // UTIL_H
