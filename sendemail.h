#ifndef SENDEMAIL_H
#define SENDEMAIL_H

#include <Wt/WString>
#include <vector>
#include <sstream>

class SendEmail
{
private:
	std::istringstream emailBuffer;
	Wt::WString login, pwd, from;
	std::vector<Wt::WString> recipients;
	bool confIsValid;

	std::string getCredentialsFileName();
	bool readConfigurationFile(Wt::WString &login, Wt::WString &pwd, Wt::WString &from, std::vector<Wt::WString> &to);

	static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp);

public:
	enum EmailType {
		PLAIN,
		HTML
	};

	SendEmail();

	bool send(const Wt::WString &title, const Wt::WString &msg, EmailType type = HTML);
};

#endif // SENDEMAIL_H
