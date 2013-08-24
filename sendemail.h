#ifndef WTCOMMENTS_SENDEMAIL_H
#define WTCOMMENTS_SENDEMAIL_H

#include <Wt/WString>

#include <sstream>
#include <vector>

class SendEmail
{
private:
	Wt::WString login, pwd, smtp_server, from;
	std::vector<Wt::WString> recipients;
	bool isEnabled, verbose;
	std::string mailHeaderToFrom, mailHeaderContentHTML, mailHeaderContentPLAIN;
	std::istringstream emailBuffer; // curl only

	std::string getCredentialsFileName();
	bool readConfigurationFile(bool &enable, bool &verbose, Wt::WString &login, Wt::WString &pwd, Wt::WString &smtp_server, Wt::WString &from, std::vector<Wt::WString> &to);

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

#endif // WTCOMMENTS_SENDEMAIL_H
