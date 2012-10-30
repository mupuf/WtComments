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
	Wt::WTextEdit *_editMsg;
	Wt::WText *_noComments;
	Wt::WBoxLayout *layout;

	std::auto_ptr<CommentsDB> db;

	std::string getValueFromEnv(const Wt::WEnvironment& env,
				const std::string &key,
				const std::string &defaultValue = std::string()) const;
	void setCommentThread(const Wt::WString &thread);
	void postComment();
	void drawComment(const Comment &comment);

public:
	View(const Wt::WEnvironment &env, Wt::WServer &server, std::string &thread);
};

#endif // VIEW_H
