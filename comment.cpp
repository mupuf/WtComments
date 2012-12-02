#include "comment.h"

Comment::Comment(const Wt::WString &author, const Wt::WString &email,
		const Wt::WString &msg, Wt::WDate date,
		Wt::WTime time, const Wt::WString &clientAddress, const Wt::WString &sessionId)
		:_author(author), _email(email), _date(date), _time(time), _msg(msg),
			_clientAddress(clientAddress), _sessionId(sessionId)
{
}

void Comment::setAuthor(const Wt::WString &author)
{
	this->_author = author;
}

void Comment::setEmail(const Wt::WString &email)
{
	this->_email = email;
}

void Comment::setMsg(const Wt::WString &msg)
{
	this->_msg = msg;
}

void Comment::setDate(const Wt::WDate &date)
{
	this->_date = date;
}

void Comment::setTime(const Wt::WTime &time)
{
	this->_time = time;
}

void Comment::setClientAddress(const Wt::WString &clientAddress)
{
	this->_clientAddress = clientAddress;
}

void Comment::setSessionId(const Wt::WString &sessionId)
{
	this->_sessionId = sessionId;
}
