#include "util.h"

/* from http://stackoverflow.com/questions/4643512/replace-substring-with-another-substring-c */
std::string &strReplace(std::string & subj, const std::string &old, const std::string &neu)
{
	size_t uiui = - neu.size();

	do
	{
		uiui = subj.find(old, uiui + neu.size());
		if (uiui != std::string::npos)
		{
			subj.erase(uiui, old.size());
			subj.insert(uiui, neu);
		}
	} while(uiui != std::string::npos);
	return subj;
}

unsigned int countOccurencies(const std::string &str, const std::string &substr)
{
	size_t  pos = 0, count = 0;
	while (pos = str.find(substr, pos) != std::string::npos) {
		count++;
		pos++;
	}
	return count;
}
