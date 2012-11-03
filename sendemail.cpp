#include "sendemail.h"

#include <stdio.h>
#include <sys/wait.h>
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

	for (size_t i = 0; i < jsonRecipients.size(); ++i)
		to.push_back(readJSONValue<Wt::WString>(jsonRecipients[i], "email", ""));

	return true;
}

SendEmail::SendEmail() : isEnabled(false)
{
	bool enable;

	if (readConfigurationFile(enable, verbose, login, pwd, smtp_server, from, recipients)) {
		isEnabled = enable;

		/* generate mail headers */
		mailHeaderToFrom = "";
		for (size_t i = 0; i < recipients.size(); ++i) {
			mailHeaderToFrom += "To: " + recipients[i].toUTF8() + "\n";
		}
		mailHeaderToFrom += "From: " + from.toUTF8() + "\n";

		mailHeaderContentHTML = "Mime-version: 1.0\n";
		mailHeaderContentHTML += "Content-Type: text/html; charset=\"UTF-8\"\n";
		mailHeaderContentHTML += "Content-Transfer-Encoding: 8bit\n";
		mailHeaderContentHTML += "\n";

		mailHeaderContentPLAIN = "Content-Type: text/plain; charset=\"UTF-8\"\n";
		mailHeaderContentPLAIN += "\n";
	}
}

size_t SendEmail::payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
	SendEmail *_this = (SendEmail *)userp;

	if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1))
		return 0;

	_this->emailBuffer.read((char *)ptr, size * nmemb);

	return _this->emailBuffer.gcount();
}

bool SendEmail::sendUsingCurl(const std::string &content)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *curl_recipients = NULL;

	emailBuffer.str(content);

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
}

bool SendEmail::sendUsingLocalMail(const std::string &content)
{
	pid_t pid;
	int mailpipe[2];
	int returnCode = 0;
	ssize_t writeLen = 0;
	ssize_t retLen = 0;
	ssize_t mailLen = content.length();

	if(pipe(mailpipe)){
		std::cerr << "sendUsingLocalMail: Failed to create pipe." << std::endl;
		return false;
	}

	pid = fork();
	if (pid == (pid_t) 0) {
		close(mailpipe[1]);
		if (dup2(mailpipe[0], STDIN_FILENO) < 0) {
			std::cerr << "sendUsingLocalMail: Failed to dup2." << std::endl;
			return false;
		}
		execl("/usr/bin/mail", "/usr/bin/mail", "-t", NULL);
		std::cerr << "sendUsingLocalMail: Failed to execl." << std::endl;
		exit(EXIT_FAILURE);
	}

	close(mailpipe[0]);

	/* write mail content to mailpipe[1] */
	while (writeLen < mailLen) {
		retLen = write(mailpipe[1], content.c_str() + writeLen, sizeof(char) * (mailLen - writeLen));
		if (retLen > 0) {
			writeLen += retLen;
		} else {
			std::cerr << "sendUsingLocalMail: Failed to write the whole mail." << std::endl;
			break;
		}
	}
	close(mailpipe[1]);

	pid = waitpid(pid, &returnCode, 0);
	if ((pid > 0) && (returnCode == 0))
		return true;

	if (pid > 0)
		std::cerr << "sendUsingLocalMail: mail command returned !=0 code." << std::endl;
	else
		std::cerr << "sendUsingLocalMail: Failed to waitpid()." << std::endl;

	return false;
}

bool SendEmail::send(const Wt::WString &title, const Wt::WString &msg, EmailType type)
{
#ifndef SEND_EMAIL
	UNUSED(title);
	UNUSED(msg);
	UNUSED(type);
	return false;
#else
	std::string mailBuffer;

	if (!isEnabled)
		return false;

	/* exit if there is no-one to send a mail to */
	if (recipients.size() < 1)
		return false;

	/* generate the mail */
	mailBuffer = mailHeaderToFrom;
	mailBuffer += "Subject: " + title.toUTF8() + "\n";
	if (type == HTML) {
		mailBuffer += mailHeaderContentHTML;
	} else {
		mailBuffer += mailHeaderContentPLAIN;
	}
	mailBuffer += msg.toUTF8();

	if (smtp_server == "local")
		return sendUsingLocalMail(mailBuffer);

	return sendUsingCurl(mailBuffer);
#endif
}
