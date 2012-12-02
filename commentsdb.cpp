#include "commentsdb.h"
#include "view.h"
#include "util.h"

#include <Wt/WServer>
#include <Wt/WApplication>
#include <Wt/Json/Object>
#include <Wt/Json/Parser>
#include <Wt/Json/Array>
#include <Wt/Json/Value>

#include <iostream>
#include <fstream>

boost::recursive_mutex CommentsDB::thread_clients_mutex;
std::map<Wt::WString, std::vector<CommentsDB::Client> > CommentsDB::url_clients;

boost::recursive_mutex CommentsDB::comments_mutex;

std::string CommentsDB::getDBFile() const
{
	std::string url = client.url.toUTF8();
	strReplace(url, "/", "|");
	return "./db/" + url + ".json";
}

std::string CommentsDB::encodeJSONString(std::string str)
{
	strReplace(str, "\"", "&#34;");
	strReplace(str, "\\", "\\\\");
	return str;
}

std::string CommentsDB::decodeJSONString(std::string str)
{
	strReplace(str, "&#34;", "\"");
	strReplace(str, "\\\\", "\\");
	return str;
}

Comment CommentsDB::readJSonComment(const Wt::Json::Object &object)
{
	const Wt::WString &author = readJSONValue<Wt::WString>(object, "author", "no_author");
	const Wt::WString &email = readJSONValue<Wt::WString>(object, "email", "");
	Wt::WDate date = Wt::WDate::fromJulianDay(readJSONValue<int>(object, "date", Wt::WDate::currentDate().toJulianDay()));
	Wt::WTime time = Wt::WTime::fromString(readJSONValue<Wt::WString>(object, "time", "00:00:00"));
	const Wt::WString &msg = readJSONValue<Wt::WString>(object, "msg", "Err: Invalid message");
	const Wt::WString &clientAddress = readJSONValue<Wt::WString>(object, "IP", "0.0.0.0");
	const Wt::WString &sessionId = readJSONValue<Wt::WString>(object, "sessionId", "0");

	std::string author_std = decodeJSONString(author.toUTF8());
	std::string email_std = decodeJSONString(email.toUTF8());
	std::string msg_std = decodeJSONString(msg.toUTF8());

	return Comment(Wt::WString::fromUTF8(author_std),
			Wt::WString::fromUTF8(email_std),
			Wt::WString::fromUTF8(msg_std), date, time,
			clientAddress, sessionId);
}

std::vector<Comment> CommentsDB::readCommentsFromFile()
{
	std::vector<Comment> comments;
	std::string file;

	/* read the file */
	std::ifstream db(getDBFile().c_str());
	if (db.is_open())
	{
		std::string line;
		while (db.good()) {
			getline(db, line);
			file += line;
		}
		db.close();
	}
	else {
		std::cerr << "The file '" + getDBFile() + "' cannot be opened" << std::endl;
		return comments;
	}

	if (file.size() == 0) {
		std::cerr << "The file '" + getDBFile() + "' is empty" << std::endl;
		return comments;
	}

	/* parse the file we read */
	try {
		Wt::Json::Object result;
		Wt::Json::parse(file, result);
		const Wt::Json::Array& jsonComments = readJSONValue<Wt::Json::Array>(result, "comments", Wt::Json::Array());

		/* parse the comments we read */
		for (size_t i = 0; i < jsonComments.size(); i++)
			comments.push_back(readJSonComment(jsonComments[i]));
	}
	catch (Wt::WException error)
	{
		std::cerr << "Error while parsing file '" << getDBFile() << "': " << error.what() << "\n" << std::endl;

		/* email */
		Wt::WString msg, title = "Error while parsing thread '{1}':";
		title = title.arg(client.url);
		msg = "{1}\n\nError = '{2}'\n\nFile = '{3}'";
		msg.arg(title).arg(error.what()).arg(file);
		sendEmail.send(title, msg, SendEmail::PLAIN);
	}

	return comments;
}

void CommentsDB::saveNewComment(const Comment &comment)
{
	boost::recursive_mutex::scoped_lock lock_comments(comments_mutex);

	/* read the current comments */
	std::vector<Comment> comments = readCommentsFromFile();

	/* add the new comment */
	comments.push_back(comment);

	/* write the comments to the file */
	std::ofstream db(getDBFile().c_str());
	if (db.is_open())
	{
		db << "{" << std::endl;
		db << "	\"comments\":" << std::endl;
		db << "	[" << std::endl;

		for (size_t i = 0; i < comments.size(); i++)
		{
			std::string author = comments[i].author().toUTF8();
			std::string email = comments[i].email().toUTF8();
			std::string msg = comments[i].msg().toUTF8();

			/* replace the " character by it's html equivalent */
			author = encodeJSONString(author);
			email = encodeJSONString(email);
			msg = encodeJSONString(msg);

			db << "		{ \"author\": \"" << author;
			db << "\", \"email\": \"" << email;
			db << "\", \"date\": " << comments[i].date().toJulianDay();
			db << ", \"time\": \"" << comments[i].time().toString();
			db << "\", \"msg\": \"" << msg;
			db << "\", \"IP\": \"" << comments[i].clientAddress();
			db << "\", \"sessionId\": \"" << comments[i].sessionId();
			db << "\" }" << std::endl << std::endl << std::endl;
		}

		db << "	]" << std::endl;
		db << "}" << std::endl;

		/* TODO: find a way to fsync! */
		db.close();
	}
}

bool CommentsDB::validateComment(const Comment &comment, Wt::WString &error) const
{
	if (comment.author().toUTF8().length() < 3) {
		error = "The author name is too short (< 3 characters)";
		return false;
	}
	if (comment.author().toUTF8().length() > 30) {
		error = "The author name is too long (> 30 characters)";
		return false;
	}

	if (comment.msg().toUTF8().length() < 30) {
		error = "The message is too short (< 30 characters)";
		return false;
	}
	if (comment.msg().toUTF8().length() > 4096) {
		error = "The message is too long (> 4096 characters)";
		return false;
	}

	if (countOccurencies(comment.msg().toUTF8(), "<a ") > 10) {
		error = "Too many links in this comment (> 10).<br/>Are you a spammer?";
		return false;
	}

	/* TODO: - Limit the number of comments per minute per IP
	 *       - Implement some kind of capcha validation
	*/


	return true;
}

CommentsDB::CommentsDB(Wt::WServer &server, const Wt::WString &url, NewCommentCallback cb)
		: _server(server)
{
	boost::recursive_mutex::scoped_lock lock(thread_clients_mutex);

	client.sessionID = Wt::WApplication::instance()->sessionId();
	client.cb = cb;
	client.url = url;

	url_clients[client.url].push_back(client);

	/* display the comments */
	std::vector<Comment> comments = readCommentsFromFile();
	for (size_t i = 0; i < comments.size(); i++)
		client.cb(comments[i]);
}

CommentsDB::~CommentsDB()
{
	boost::recursive_mutex::scoped_lock lock(thread_clients_mutex);

	/* erase the client from the client DB */
	std::vector<Client> clients = url_clients[client.url];
	for (size_t i = 0; i < clients.size(); i++) {
		if (client.sessionID == clients[i].sessionID) {
			url_clients[client.url].erase(url_clients[client.url].begin() + i);
			return;
		}
	}
}

bool CommentsDB::postComment(const Comment &comment, Wt::WString &error)
{
	boost::recursive_mutex::scoped_lock lock_thread(thread_clients_mutex);
	boost::recursive_mutex::scoped_lock lock_comments(comments_mutex);

	if (!validateComment(comment, error))
		return false;

	/* add the comment to the DB */
	saveNewComment(comment);

	/* warn the other clients that there is a new comment */
	std::vector<Client> clients = url_clients[client.url];

	for (size_t i = 0; i < clients.size(); i++) {
		if (client.sessionID == clients[i].sessionID)
			clients[i].cb(comment);
		else
			_server.post(clients[i].sessionID, boost::bind(clients[i].cb, comment));
	}

	/* email */
	Wt::WString msg = "<p>Hi MuPuF.org users!</p><p>There is a new comment from '{1}' on article <a href=\"{2}\">{2}</a>:</p>------------------------------{3}";
	msg = msg.arg(comment.author()).arg(client.url).arg(comment.msg());
	sendEmail.send("New comment on " + client.url, msg);

	return true;
}
