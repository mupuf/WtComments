#include "unsubscribe.h"

#include <Wt/WRegExpValidator>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WTemplate>
#include <Wt/WMessageBox>
#include <Wt/WOverlayLoadingIndicator>

void Unsubscribe::drawComment(const Comment &)
{

}

void Unsubscribe::unsubscribe()
{
	Wt::WString error;
	if (db->unsubscribe(_editEmail->text().toUTF8(), error)) {
		Wt::WMessageBox::show("Unsubscription taken into account",
				"Don't hesitate to tell us why you unsubscribed!", Wt::Ok);
		_editEmail->setText("");
	} else
		Wt::WMessageBox::show("An error occured while unsubscribing ",
				error, Wt::Ok);
}

Unsubscribe::Unsubscribe(const Wt::WEnvironment& env, Wt::WServer &server, std::string &url) :
	Wt::WApplication(env)
{
	Wt::WApplication *app = Wt::WApplication::instance();

	app->setLoadingIndicator(new Wt::WOverlayLoadingIndicator());

	/* Set the title */
	setTitle("Unsubscribe to comments for the thread \"" + url + "\"");

	Wt::WPushButton *button = new Wt::WPushButton("Unsubscribe");
	button->setId("btn_unsub");
	app->styleSheet().addRule("#btn_unsub", "float: right;");
	button->clicked().connect(this, &Unsubscribe::unsubscribe);

	_editEmail = new Wt::WLineEdit();
	Wt::WRegExpValidator *emailVal = new Wt::WRegExpValidator("^([a-zA-Z0-9_\\-\\.]+)@((\\[[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.)|(([a-zA-Z0-9\\-]+\\.)+))([a-zA-Z]{2,4}|[0-9]{1,3})(\\]?)$", _editEmail);
	_editEmail->setMinimumSize(Wt::WLength(200), _editEmail->minimumHeight());
	_editEmail->setValidator(emailVal);

	Wt::WTemplate *t = new Wt::WTemplate();
	t->setTemplateText("" \
		"<div>"
			"<h3>Unsubscribe to comments from thread '" + url + "'</h3>" \
			"<p><label>Email: ${email-edit}</label></p>" \
			"${unsub_btn}"
		"</div>"
	);
	t->setId("unsubscribe");
	app->styleSheet().addRule("#unsubscribe", "width: 500px; margin: auto;");
	t->bindWidget("email-edit", _editEmail);
	t->bindWidget("unsub_btn", button);
	root()->addWidget(t);

	/* Init the DB */
	db.reset(new CommentsDB(server, url, boost::bind(&Unsubscribe::drawComment, this, _1)));
}
