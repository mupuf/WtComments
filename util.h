#ifndef UTIL_H
#define UTIL_H
	#include <string>

	std::string &strReplace(std::string & subj, const std::string &old, const std::string &neu);
	unsigned int countOccurencies(const std::string &str, const std::string &substr);
#endif // UTIL_H
