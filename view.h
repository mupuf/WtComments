#ifndef WTCOMMENTS_VIEW_H
#define WTCOMMENTS_VIEW_H

#include "commentsdb.h"

#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WVBoxLayout>

#include <memory>

class View : public Wt::WApplication
{
private:
	Wt::WLineEdit *_editAuthor;
	Wt::WLineEdit *_editEmail;
#ifdef USE_SIMPLE_EDITOR
	Wt::WTextArea *_editMsg;
#else
	Wt::WTextEdit *_editMsg;
#endif
	Wt::WText *_noComments;
	Wt::WBoxLayout *layout;

	std::auto_ptr<CommentsDB> db;

	void postComment();
	void drawComment(const Comment &comment);

public:
	View(const Wt::WEnvironment &env, Wt::WServer &server, std::string &url);
};

#endif // WTCOMMENTS_VIEW_H
