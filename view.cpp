#include "view.h"

#include "util.h"

#include <Wt/Utils>
#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WDate>
#include <Wt/WLength>
#include <Wt/WLineEdit>
#include <Wt/WMessageBox>
#include <Wt/WOverlayLoadingIndicator>
#include <Wt/WPanel>
#include <Wt/WPushButton>
#include <Wt/WRegExpValidator>
#include <Wt/WTemplate>
#include <Wt/WText>

#ifdef USE_SIMPLE_EDITOR
#include <Wt/WTextArea>
#else
#include <Wt/WTextEdit>
#endif

void View::drawComment(const Comment &comment)
{
	Wt::WString title = Wt::WString("<span class=\"comment_author\">{1}</span> on {2} {3} said:");
	title = title.arg(Wt::Utils::htmlEncode(comment.author())).arg(comment.date().toString("dddd MMMM d yyyy")).arg(comment.time().toString());

	Wt::WPanel *panel = new Wt::WPanel();
	panel->setTitleBar(true);
	panel->setTitle(title);
	panel->setCentralWidget(new Wt::WText(comment.msg()));
	layout->addWidget(panel);

	/* hide the no comments text */
	_noComments->hide();

	Wt::WApplication::instance()->triggerUpdate();
}

void View::postComment()
{
	Comment comment(_editAuthor->text(), _editEmail->text(), _editMsg->text());
	Wt::WString error;

	if (db->postComment(comment, error)) {
		/* reset the text */
		_editMsg->setText(Wt::WString());
	} else {
		Wt::WMessageBox::show("Error while posting the comment",
				error, 	Wt::Ok);
	}
}

View::View(const Wt::WEnvironment& env, Wt::WServer &server, std::string &url) :
	Wt::WApplication(env)
{
	Wt::WApplication *app = Wt::WApplication::instance();
	app->enableUpdates(true);
	app->setLoadingIndicator(new Wt::WOverlayLoadingIndicator());

	/* Application root fixup */
	std::cerr << "Application root = '" << Wt::WApplication::appRoot()
		  << "' and doc root = '" << Wt::WApplication::docRoot()
		  << "'" << std::endl;

	/* Set the title */
	setTitle("Comments for the thread \"" + url + "\"");

	/* CSS fixups */
	app->styleSheet().addRule(".comment_author", "font-weight:bold");
	app->styleSheet().addRule("p", "margin-top: 8px");
	app->styleSheet().addRule("p", "margin-bottom: 8px");

	/* Comments section */
	Wt::WContainerWidget *w = new Wt::WContainerWidget(root());
	layout = new Wt::WBoxLayout(Wt::WBoxLayout::TopToBottom);
	w->setLayout(layout);

	/* Add a text when no comment is available */
	_noComments = new Wt::WText("<center><h2>No comments yet</h2></center>");
	_noComments->setId("no_comment");
	app->styleSheet().addRule("#no_comment", "color: lightgrey;");
	root()->addWidget(_noComments);

	/* Add a new comment */
	Wt::WPushButton *button = new Wt::WPushButton("Send");
	button->setId("btn_send");
	app->styleSheet().addRule("#btn_send", "float: right;");
	button->clicked().connect(this, &View::postComment);
	_editAuthor = new Wt::WLineEdit();
	_editEmail = new Wt::WLineEdit();
	Wt::WRegExpValidator *emailVal = new Wt::WRegExpValidator("^([a-zA-Z0-9_\\-\\.]+)@((\\[[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.)|(([a-zA-Z0-9\\-]+\\.)+))([a-zA-Z]{2,4}|[0-9]{1,3})(\\]?)$", _editEmail);
	_editEmail->setMinimumSize(Wt::WLength(200), _editEmail->minimumHeight());
	_editEmail->setValidator(emailVal);
#ifdef USE_SIMPLE_EDITOR
	_editMsg = new Wt::WTextArea();
	_editMsg->setColumns(80);
#else
	_editMsg = new Wt::WTextEdit();
	_editMsg->setConfigurationSetting(std::string("theme_advanced_statusbar_location"), std::string("none"));
	_editMsg->setHeight(250);
	_editMsg->setToolBar(0, "bold, italic, underline, |, fontsizeselect, |, justifyleft, justifycenter, justifyright, justifyfull, |, indent, outdent, |, numlist, bullist, |,link, image, blockquote, code");
#endif
	Wt::WTemplate *t = new Wt::WTemplate();
	t->setTemplateText("<hr width=\"80%\" />" \
		"<div>"
			"<h3>Send a comment</h3>" \
			"<p><label>Author: ${author-edit}</label></p>" \
			"<p><label>Notification email (optional): ${email-edit}</label></p>" \
			"${text-edit}" \
			"${send_btn}"
		"</div>"
	);

	t->setId("send_comment");
	app->styleSheet().addRule("#send_comment", "width: 90%; margin: auto;");
	t->bindWidget("author-edit", _editAuthor);
	t->bindWidget("email-edit", _editEmail);
	t->bindWidget("text-edit", _editMsg);
	t->bindWidget("send_btn", button);

	root()->addWidget(t);

	/* Init the DB */
	db.reset(new CommentsDB(server, url, boost::bind(&View::drawComment, this, _1)));
}
