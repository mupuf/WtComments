#ifndef WTCOMMENTS_H
#define WTCOMMENTS_H

#include <Wt/WApplication>
#include <Wt/WVBoxLayout>

class WtComments : public Wt::WApplication
{
private:
	Wt::WLineEdit *_editAuthor;
	Wt::WTextEdit *_editText;
	Wt::WBoxLayout *layout;

	std::string getValueFromEnv(const Wt::WEnvironment& env,
				const std::string &key,
				const std::string &defaultValue = std::string()) const;
	void setCommentThread(std::string thread);
	void drawComment(const Wt::WString &author, int julianDay, const Wt::WString &text);
	void postComment();

public:
	WtComments(const Wt::WEnvironment& env);
};

#endif // WTCOMMENT_H
