#pragma once

#include "commentsdb.h"

#include <Wt/WApplication>
#include <Wt/WEnvironment>

#include <memory>

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
