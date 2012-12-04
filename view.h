#ifndef VIEW_H
#define VIEW_H

#include <Wt/WApplication>
#include <Wt/WVBoxLayout>
#include <Wt/WEnvironment>

#include "commentsdb.h"

#include <memory>

class View : public Wt::WApplication
{
private:
	Wt::WLineEdit *_editAuthor;
	Wt::WLineEdit *_editEmail;
	Wt::WTextEdit *_editMsg;
	Wt::WText *_noComments;
	Wt::WBoxLayout *layout;

	std::auto_ptr<CommentsDB> db;

	void postComment();
	void drawComment(const Comment &comment);

public:
	View(const Wt::WEnvironment &env, Wt::WServer &server, std::string &url);
};

#endif // VIEW_H
