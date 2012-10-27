#include "comment.h"

Comment::Comment(const Wt::WString &author, const Wt::WString &msg, Wt::WDate date)
		:_author(author), _date(date), _msg(msg)
{
}

void Comment::setAuthor(const Wt::WString &author)
{
	this->_author = author;
}

void Comment::setMsg(const Wt::WString &msg)
{
	this->_msg = msg;
}

void Comment::setDate(const Wt::WDate &date)
{
	this->_date = date;
}
