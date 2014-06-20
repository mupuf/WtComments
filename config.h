#pragma once

#include <Wt/WString>

#include <sstream>
#include <vector>

class Config
{
public:
	static Config* getConfig();
	bool readConfigFile();
	std::string getConfigFileName();

	std::string websiteName();
	std::string websiteURL();
	bool emailsEnabled();
	Wt::WString login();
	Wt::WString pwd();
	Wt::WString smtp_server();
	Wt::WString from();
	std::vector<Wt::WString> recipients();
	bool isEnabled();
	bool verbose();

private:
	Config() {};
	Config(Config const &) {};
	void operator=(Config const &) {};

	std::string _websiteName, _websiteURL;
	bool _emailsEnabled;
	Wt::WString _login, _pwd, _smtp_server, _from;
	std::vector<Wt::WString> _recipients;
	bool _isEnabled, _verbose;
};
