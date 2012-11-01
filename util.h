#ifndef UTIL_H
#define UTIL_H
	#include <Wt/Json/Object>
	#include <string>
	#include <iostream>

	#define UNUSED(expr) do { (void)(expr); } while (0)

	template <class type_t>
	type_t readJSONValue(Wt::Json::Object result, const std::string &key, type_t defaultValue) {
		try {
			return result.get(key);
		} catch (Wt::WException error) {
			std::cerr << "Attribute '" << key << "' is invalid: " << error.what() << std::endl;
			return defaultValue;
		}
	}

	std::string &strReplace(std::string & subj, const std::string &old, const std::string &neu);
	unsigned int countOccurencies(const std::string &str, const std::string &substr);
	std::string getExeDirectory();
#endif // UTIL_H
