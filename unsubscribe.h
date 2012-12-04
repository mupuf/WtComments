#ifndef UNSUBSCRIBE_H
#define UNSUBSCRIBE_H

#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <memory>

#include "commentsdb.h"

class Unsubscribe : public Wt::WApplication
{
private:
	Wt::WLineEdit *_editEmail;
	std::auto_ptr<CommentsDB> db;

	void unsubscribe();
	void drawComment(const Comment &comment);

public:
	Unsubscribe(const Wt::WEnvironment &env, Wt::WServer &server, std::string &url);
};

#endif // UNSUBSCRIBE_H
