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

	Wt::WString websiteName();
	Wt::WString websiteURL();
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

	Wt::WString _websiteName, _websiteURL, _login, _pwd, _smtp_server, _from;
	std::vector<Wt::WString> _recipients;
	bool _emailsEnabled, _isEnabled, _verbose;
};
