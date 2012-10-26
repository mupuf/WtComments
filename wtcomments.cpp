#include "wtcomments.h"

#include <Wt/WLength>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WEnvironment>
#include <Wt/WPanel>
#include <Wt/WTextEdit>
#include <Wt/WDate>
#include <Wt/WTemplate>
#include <Wt/WMessageBox>

std::string WtComments::getValueFromEnv(const Wt::WEnvironment& env,
		const std::string &key,
		const std::string &defaultValue) const
{
	std::vector<std::string> param = env.getParameterValues(key);
	if (param.size() > 0) {
		return param[0];
	} else
		return defaultValue;
}

void WtComments::setCommentThread(std::string thread)
{
	setTitle("Comments for the thread \"" + thread + "\"");
}

void WtComments::drawComment(const Wt::WString &author,
				int julianDay,
				const Wt::WString &text)
{
	Wt::WString title = Wt::WString("Posted the ");
	title += Wt::WDate::fromJulianDay(julianDay).toString();
	title += Wt::WString(" by ");
	title += Wt::WString(author);

	Wt::WPanel *panel = new Wt::WPanel();
	panel->setTitleBar(true);
	panel->setTitle(title);
	panel->setCentralWidget(new Wt::WText(text));
	layout->addWidget(panel);
}

void WtComments::postComment()
{
	Wt::WString author = _editAuthor->text();
	Wt::WString text = _editText->text();

	if (author.value().length() == 0) {
		Wt::WMessageBox::show("Error while posting the comment",
				"The author name is missing",
				Wt::Ok);
		return;
	}

	if (text.value().length() < 10) {
		Wt::WMessageBox::show("Error while posting the comment",
				"The text submited is too short.<br/><br/>Please write some text",
				Wt::Ok);
		return;
	}

	/* reset the text */
	_editText->setText(Wt::WString());

	drawComment(author, Wt::WDate::currentServerDate().toJulianDay(), text);
}

WtComments::WtComments(const Wt::WEnvironment& env) :
	Wt::WApplication(env)
{
	setCommentThread(getValueFromEnv(env, "thread", "default"));

	/* Comments section */
	Wt::WContainerWidget *w = new Wt::WContainerWidget(root());
	layout = new Wt::WBoxLayout(Wt::WBoxLayout::TopToBottom);
	w->setLayout(layout);

	/* Add a new comment */
	Wt::WPushButton *button = new Wt::WPushButton("Send");
	button->clicked().connect(this, &WtComments::postComment);
	_editAuthor = new Wt::WLineEdit();
	_editText = new Wt::WTextEdit();
	_editText->setHeight(300);

	Wt::WTemplate *t = new Wt::WTemplate();
	t->setTemplateText("<hr width=\"75%\" />" \
		"<div style=\"margin-left: auto; margin-right: auto; width: 50%\">"
			"<h3>Add a new comment</h3>" \
			"<p><label>Author: ${author-edit}</label></p>" \
			"${text-edit}" \
			"${send_btn}"
		"</div>"
	);
	t->bindWidget("author-edit", _editAuthor);
	t->bindWidget("text-edit", _editText);
	t->bindWidget("send_btn", button);

	root()->addWidget(t);

}

