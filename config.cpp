#include "config.h"

#include "util.h"

#include <Wt/Json/Array>
#include <Wt/Json/Object>
#include <Wt/Json/Parser>
#include <Wt/Json/Value>

#include <fstream>
#include <iostream>

#include <stdio.h>
#include <string.h>
#include <sys/wait.h>


Config* Config::getConfig()
{
	static Config config;
	return &config;
}

std::string Config::getConfigFileName()
{
	return "./wt_comments_config.json";
}

bool Config::readConfigFile()
{
	/* Read the file */
	std::string file;
	std::ifstream db(getConfigFileName().c_str());
	if (db.is_open())
	{
		std::string line;
		while (db.good()) {
			getline(db, line);
			file += line;
		}
		db.close();
	}
	else {
		std::cerr << "The file '" + getConfigFileName() + "' cannot be opened" << "\n";
		return false;
	}

	/* Parse the file we've just read */
	Wt::Json::Object result;
	try {
		Wt::Json::parse(file, result);
	} catch (Wt::Json::ParseError e) {
		std::string error = e.what();
		std::cerr << "Error while parsing the configuration file: " + error << "\n";
		return false;
	}

	_isEnabled = readJSONValue<bool>(result, "enable", false);
	_verbose = readJSONValue<bool>(result, "verbose", false);
	_login = readJSONValue<Wt::WString>(result, "login", "");
	_pwd = readJSONValue<Wt::WString>(result, "pwd", "");
	_smtp_server = readJSONValue<Wt::WString>(result, "smtp_server", "");
	_from = readJSONValue<Wt::WString>(result, "from", "");

	Wt::Json::Array jsonRecipients = readJSONValue<Wt::Json::Array>(result, "to", Wt::Json::Array());

	for (size_t i = 0; i < jsonRecipients.size(); ++i)
		_recipients.push_back(readJSONValue<Wt::WString>(jsonRecipients[i], "email", ""));

	return true;
}

std::string Config::websiteName()
{
	return _websiteName;
}

std::string Config::websiteURL()
{
	return _websiteURL;
}

bool Config::emailsEnabled()
{
	return _emailsEnabled;
}

Wt::WString Config::login()
{
	return _login;
}

Wt::WString Config::pwd()
{
	return _pwd;
}

Wt::WString Config::smtp_server()
{
	return _smtp_server;
}

Wt::WString Config::from()
{
	return _from;
}

std::vector<Wt::WString> Config::recipients()
{
	return _recipients;
}

bool Config::isEnabled()
{
	return _isEnabled;
}

bool Config::verbose()
{
	return _verbose;
}
