#pragma once

#include <Wt/WApplication>
#include <Wt/WDate>
#include <Wt/WDate>
#include <Wt/WEnvironment>
#include <Wt/WString>
#include <Wt/WTime>

class Comment
{
private:
	Wt::WString _author;
	Wt::WString _email;
	Wt::WDate _date;
	Wt::WTime _time;
	Wt::WString _msg;
	Wt::WString _clientAddress;
	Wt::WString _sessionId;
public:
	Comment() { };
	Comment(const Wt::WString &author, const Wt::WString &email,
		const Wt::WString &msg,
		Wt::WDate date = Wt::WDate::currentServerDate(),
		Wt::WTime time = Wt::WTime::currentServerTime(),
		const Wt::WString &clientIP = Wt::WApplication::instance()->environment().clientAddress(),
		const Wt::WString &sessionId = Wt::WApplication::instance()->sessionId());

	Wt::WString author() const { return _author; };
	void setAuthor(const Wt::WString &author);

	Wt::WString email() const { return _email; };
	void setEmail(const Wt::WString &email);

	Wt::WString msg() const { return _msg; };
	void setMsg(const Wt::WString &msg);

	Wt::WDate date() const { return _date; };
	void setDate(const Wt::WDate &date);

	Wt::WTime time() const { return _time; };
	void setTime(const Wt::WTime &time);

	Wt::WString clientAddress() const { return _clientAddress; };
	void setClientAddress(const Wt::WString &clientAddress);

	Wt::WString sessionId() const { return _sessionId; };
	void setSessionId(const Wt::WString &sessionId);
};
