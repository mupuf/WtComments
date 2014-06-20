#pragma once

#include <Wt/WString>

#include <sstream>
#include <vector>

class SendEmail
{
private:
	std::string mailHeaderToFrom, mailHeaderContentHTML, mailHeaderContentPLAIN;
	std::istringstream emailBuffer; // curl only

	static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp);

	bool sendUsingLocalMail(const std::string &content);
	bool sendUsingCurl(const std::string &content);

public:
	enum EmailType {
		PLAIN,
		HTML
	};

	SendEmail();

	bool send(const Wt::WString &title, const Wt::WString &msg, EmailType type = HTML,
		  const std::vector<std::string> &recipients = std::vector<std::string>(),
		  bool warnAdmins=true);
};
