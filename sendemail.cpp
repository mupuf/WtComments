#include "sendemail.h"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <curl/curl.h>

#include <Wt/Json/Object>
#include <Wt/Json/Parser>
#include <Wt/Json/Array>
#include <Wt/Json/Value>

#include "util.h"

/* This file is mostly copied from http://curl.haxx.se/libcurl/c/smtp-tls.html */

std::string SendEmail::getCredentialsFileName()
{
	return "./wt_comments_email.json";
}

bool SendEmail::readConfigurationFile(bool &enable, bool &verbose,
		Wt::WString &login, Wt::WString &pwd, Wt::WString &smtp_server,
		Wt::WString &from, std::vector<Wt::WString> &to)
{
	Wt::Json::Array jsonRecipients;
	std::string file;

	/* read the file */
	std::ifstream db(getCredentialsFileName().c_str());
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
		std::cerr << "The file '" + getCredentialsFileName() + "' cannot be opened" << std::endl;
		return false;
	}

	/* parse the file we read */
	Wt::Json::Object result;
	Wt::Json::parse(file, result);

	enable = readJSONValue<bool>(result, "enable", false);
	verbose = readJSONValue<bool>(result, "verbose", false);
	login = readJSONValue<Wt::WString>(result, "login", "");
	pwd = readJSONValue<Wt::WString>(result, "pwd", "");
	smtp_server = readJSONValue<Wt::WString>(result, "smtp_server", "");
	from = readJSONValue<Wt::WString>(result, "from", "");
	jsonRecipients = readJSONValue<Wt::Json::Array>(result, "to", Wt::Json::Array());

	for (size_t i = 0; i < jsonRecipients.size(); i++)
		to.push_back(readJSONValue<Wt::WString>(jsonRecipients[i], "email", ""));

	return true;
}

SendEmail::SendEmail() : isEnabled(false)
{
	bool enable;

	if (readConfigurationFile(enable, verbose, login, pwd, smtp_server, from, recipients))
		isEnabled = enable;
}

size_t SendEmail::payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
	SendEmail *_this = (SendEmail *)userp;

	if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1))
		return 0;

	_this->emailBuffer.read((char *)ptr, size * nmemb);

	return _this->emailBuffer.gcount();
}

bool SendEmail::send(const Wt::WString &title, const Wt::WString &msg, EmailType type)
{
#ifndef SEND_EMAIL
	UNUSED(title);
	UNUSED(msg);
	UNUSED(type);
	return false;
#else
	CURL *curl;
	CURLcode res;
	struct curl_slist *curl_recipients = NULL;
	std::string buffer;

	if (!isEnabled)
		return false;

	/* exit if there is no-one to send a mail to */
	if (recipients.size() < 1)
		return false;

	/* generate the mail */
	for (size_t i = 0; i < recipients.size(); i++)
		buffer += "To: " + recipients[i].toUTF8() + "\n";
	buffer += "From: " + from.toUTF8() + "\n",
	buffer += "Subject: " + title.toUTF8() + "\n";
	if (type == HTML) {
		buffer += "Mime-version: 1.0\n";
		buffer += "Content-Type: text/html; charset=\"UTF-8\"\n";
		buffer += "Content-Transfer-Encoding: 8bit\n";
		buffer += "\n";
	} else
		buffer += "Content-Type: text/plain; charset=\"utf-8\"\n";
	buffer += msg.toUTF8();

	emailBuffer.str(buffer);

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, smtp_server.toUTF8().c_str());
		curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
		curl_easy_setopt(curl, CURLOPT_USERNAME, login.toUTF8().c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, pwd.toUTF8().c_str());
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from.toUTF8().c_str());
		for (size_t i = 0; i < recipients.size(); i++)
			curl_recipients = curl_slist_append(curl_recipients, recipients[i].toUTF8().c_str());
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, curl_recipients);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, this);

		/* Verbose ? Set to 1 if you want to debug stuff */
		curl_easy_setopt(curl, CURLOPT_VERBOSE, verbose?1L:0L);

		/* send the message (including headers) */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
		    curl_easy_strerror(res));

		/* free the list of recipients and clean up */
		curl_slist_free_all(curl_recipients);
		curl_easy_cleanup(curl);
	}

	return true;
#endif
}
