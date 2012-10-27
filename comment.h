#ifndef COMMENT_H
#define COMMENT_H

#include <Wt/WString>
#include <Wt/WDate>
#include <Wt/WDate>

class Comment
{
private:
	Wt::WString _author;
	Wt::WDate _date;
	Wt::WString _msg;
public:
	Comment() { };
	Comment(const Wt::WString &author, const Wt::WString &msg,
		Wt::WDate date = Wt::WDate::currentServerDate());

	Wt::WString author() const { return _author; };
	void setAuthor(const Wt::WString &author);

	Wt::WString msg() const { return _msg; };
	void setMsg(const Wt::WString &msg);

	Wt::WDate date() const { return _date; };
	void setDate(const Wt::WDate &date);
};

#endif // COMMENT_H
