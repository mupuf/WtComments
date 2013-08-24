#include "sendemail.h"

#include "config.h"
#include "util.h"

#include <Wt/Json/Array>
#include <Wt/Json/Object>
#include <Wt/Json/Parser>
#include <Wt/Json/Value>

#include <fstream>
#include <iostream>

#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

/* This file is mostly copied from http://curl.haxx.se/libcurl/c/smtp-tls.html */

SendEmail::SendEmail(): mailHeaderToFrom(""), mailHeaderContentHTML(""), mailHeaderContentPLAIN("")
{
	/* Generate mail headers */
	Config* conf = Config::getConfig();
	size_t recipientsCount = conf->recipients().size();
	for (size_t i = 0; i < recipientsCount; ++i) {
		mailHeaderToFrom += "Bcc: " + conf->recipients()[i].toUTF8() + "\n";
	}
	mailHeaderToFrom += "From: " + conf->from().toUTF8() + "\n";

	mailHeaderContentHTML = "Mime-version: 1.0\n";
	mailHeaderContentHTML += "Content-Type: text/html; charset=\"UTF-8\"\n";
	mailHeaderContentHTML += "Content-Transfer-Encoding: 8bit\n";
	mailHeaderContentHTML += "\n";

	mailHeaderContentPLAIN = "Content-Type: text/plain; charset=\"UTF-8\"\n";
	mailHeaderContentPLAIN += "\n";
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
#ifndef SEND_EMAIL
	UNUSED(content);
	return false;
#else
	CURL *curl;
	CURLcode res;
	struct curl_slist *curl_recipients = NULL;
	Config* conf = Config::getConfig();

	emailBuffer.str(content);

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, conf->smtp_server().toUTF8().c_str());
		curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
		curl_easy_setopt(curl, CURLOPT_USERNAME, conf->login().toUTF8().c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, conf->pwd().toUTF8().c_str());
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, conf->from().toUTF8().c_str());
		for (size_t i = 0; i < conf->recipients().size(); i++)
			curl_recipients = curl_slist_append(curl_recipients, conf->recipients()[i].toUTF8().c_str());
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, curl_recipients);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(curl, CURLOPT_READDATA, this);
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		/* Verbose ? Set to 1 if you want to debug stuff */
		curl_easy_setopt(curl, CURLOPT_VERBOSE, conf->verbose()?1L:0L);

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

bool SendEmail::sendUsingLocalMail(const std::string &content)
{
#ifndef SEND_EMAIL
	UNUSED(content);
	return false;
#else
	pid_t pid;
	int mailpipe[2];
	int returnCode = 0;
	ssize_t writeLen = 0;
	ssize_t retLen = 0;
	ssize_t mailLen = content.length();

	if(pipe(mailpipe)){
		std::cerr << "sendUsingLocalMail: Failed to create pipe.\n";
		return false;
	}

	pid = fork();
	if (pid == (pid_t) 0) {
		close(mailpipe[1]);
		if (dup2(mailpipe[0], STDIN_FILENO) < 0) {
			std::cerr << "sendUsingLocalMail: Failed to dup2.\n";
			return false;
		}
		execl("/usr/bin/mail", "/usr/bin/mail", "-t", NULL);
		std::cerr << "sendUsingLocalMail: Failed to execl.\n";
		exit(EXIT_FAILURE);
	}

	close(mailpipe[0]);

	/* write mail content to mailpipe[1] */
	while (writeLen < mailLen) {
		retLen = write(mailpipe[1], content.c_str() + writeLen, sizeof(char) * (mailLen - writeLen));
		if (retLen > 0) {
			writeLen += retLen;
		} else {
			std::cerr << "sendUsingLocalMail: Failed to write the whole mail.\n";
			break;
		}
	}
	close(mailpipe[1]);

	pid = waitpid(pid, &returnCode, 0);
	if ((pid > 0) && (returnCode == 0))
		return true;

	if (pid > 0)
		std::cerr << "sendUsingLocalMail: mail command returned !=0 code.\n";
	else
		std::cerr << "sendUsingLocalMail: Failed to waitpid().\n";

	return false;
#endif
}

bool SendEmail::send(const Wt::WString &title, const Wt::WString &msg, EmailType type, const std::vector<std::string> &recipients, bool warnAdmins)
{
#ifndef SEND_EMAIL
	UNUSED(title);
	UNUSED(msg);
	UNUSED(type);
	UNUSED(recipients);
	UNUSED(warnAdmins);
	return false;
#else
	std::string mailBuffer;
	Config* conf = Config::getConfig();

	if (!conf->isEnabled())
		return false;

	/* generate the mail */
	size_t recipientsCount = recipients.size();
	for (size_t i = 0; i < recipientsCount; ++i)
		mailBuffer += "Bcc: " + recipients[i] + "\n";

	if (warnAdmins)
		mailBuffer += mailHeaderToFrom;
	else
		mailBuffer += "From: " + conf->from().toUTF8() + "\n";

	mailBuffer += "Subject: " + title.toUTF8() + "\n";
	if (type == HTML) {
		mailBuffer += mailHeaderContentHTML;
	} else {
		mailBuffer += mailHeaderContentPLAIN;
	}
	mailBuffer += msg.toUTF8();

	if (conf->smtp_server() == "local")
		return sendUsingLocalMail(mailBuffer);
	else
		return sendUsingCurl(mailBuffer);
#endif
}
