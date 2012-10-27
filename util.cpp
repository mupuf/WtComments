#include "util.h"

/* from http://stackoverflow.com/questions/4643512/replace-substring-with-another-substring-c */
std::string &strReplace(std::string & subj, const std::string &old, const std::string &neu)
{
	size_t uiui;

	do
	{
		uiui = subj.find(old);
		if (uiui != std::string::npos)
		{
			subj.erase(uiui, old.size());
			subj.insert(uiui, neu);
		}
	} while(uiui != std::string::npos);
	return subj;
}
