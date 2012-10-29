#include "view.h"

#include <Wt/WLength>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WPanel>
#include <Wt/WTextEdit>
#include <Wt/WDate>
#include <Wt/WTemplate>
#include <Wt/WMessageBox>
#include <Wt/WApplication>
#include <Wt/WOverlayLoadingIndicator>

#include "util.h"

void View::setCommentThread(const Wt::WString &thread)
{
	setTitle("Comments for the thread \"" + thread + "\"");
}

void View::drawComment(const Comment &comment)
{
	Wt::WString title = Wt::WString("<span class=\"comment_author\">{1}</span> on {2} {3} said:");
	title = title.arg(comment.author()).arg(comment.date().toString("dddd MMMM d yyyy")).arg(comment.time().toString());

	Wt::WPanel *panel = new Wt::WPanel();
	panel->setTitleBar(true);
	panel->setTitle(title);
	panel->setCentralWidget(new Wt::WText(comment.msg()));
	layout->addWidget(panel);

	Wt::WApplication::instance()->triggerUpdate();
}

void View::postComment()
{
	Comment comment(_editAuthor->text(), _editMsg->text());

	if (_editAuthor->text().value().length() == 0) {
		Wt::WMessageBox::show("Error while posting the comment",
				"The author name is missing",
				Wt::Ok);
		return;
	}

	if (_editMsg->text().value().length() < 10) {
		Wt::WMessageBox::show("Error while posting the comment",
				"The text submited is too short.<br/><br/>Please write some text",
				Wt::Ok);
		return;
	}

	/* reset the text */
	_editMsg->setText(Wt::WString());

	db->postComment(comment);
}

View::View(const Wt::WEnvironment& env, Wt::WServer &server, std::string &thread) :
	Wt::WApplication(env)
{
	Wt::WApplication *app = Wt::WApplication::instance();
	app->enableUpdates(true);
	app->setLoadingIndicator(new Wt::WOverlayLoadingIndicator());

	/* Application root fixup */
	std::cerr << "Application root = '" << Wt::WApplication::appRoot()
		  << "' and doc root = '" << Wt::WApplication::docRoot()
		  << "'" << std::endl;

	/* fixup the thread to get rid of any / */
	strReplace(thread, "/", "|");
	setCommentThread(thread);

	/* CSS fixups */
	app->styleSheet().addRule(".comment_author", "font-weight:bold");
	app->styleSheet().addRule("p", "margin: 0px");

	/* Comments section */
	Wt::WContainerWidget *w = new Wt::WContainerWidget(root());
	layout = new Wt::WBoxLayout(Wt::WBoxLayout::TopToBottom);
	w->setLayout(layout);

	/* Add a new comment */
	Wt::WPushButton *button = new Wt::WPushButton("Send");
	button->clicked().connect(this, &View::postComment);
	_editAuthor = new Wt::WLineEdit();
	_editMsg = new Wt::WTextEdit();
	_editMsg->setHeight(300);

	Wt::WTemplate *t = new Wt::WTemplate();
	t->setTemplateText("<hr width=\"75%\" />" \
		"<div style=\"margin-left: auto; margin-right: auto; width: 50%\">"
			"<h3>Send a comment</h3>" \
			"<p><label>Author: ${author-edit}</label></p>" \
			"${text-edit}" \
			"${send_btn}"
		"</div>"
	);
	t->bindWidget("author-edit", _editAuthor);
	t->bindWidget("text-edit", _editMsg);
	t->bindWidget("send_btn", button);

	root()->addWidget(t);

	/* Init the DB */
	db.reset(new CommentsDB(server, thread, boost::bind(&View::drawComment, this, _1)));
}

