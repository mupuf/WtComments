#include "commentsdb.h"

#include "config.h"
#include "util.h"
#include "view.h"

#include <Wt/Json/Array>
#include <Wt/Json/Object>
#include <Wt/Json/Parser>
#include <Wt/Json/Value>
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WServer>

#include <fstream>
#include <iostream>
#include <set>

boost::recursive_mutex CommentsDB::thread_clients_mutex;
std::map<Wt::WString, std::vector<CommentsDB::Client> > CommentsDB::url_clients;

boost::recursive_mutex CommentsDB::comments_mutex;

std::string CommentsDB::getDBFile() const
{
	std::string url = client.thread.toUTF8();

	if(url.find("http://") == 0) {
		url = url.substr(7);
	} else if (url.find("https://") == 0) {
		url = url.substr(8);
	}

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

std::string CommentsDB::readJSonUnsub(const Wt::Json::Object &object)
{
	const Wt::WString &email = readJSONValue<Wt::WString>(object, "email", "");

	return decodeJSONString(email.toUTF8());
}

bool CommentsDB::parseFile(std::vector<Comment> &comments, std::vector<std::string> &unsubscribers)
{
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
		std::cerr << "The file '" + getDBFile() + "' cannot be opened" << '\n';
		return false;
	}

	if (file.size() == 0) {
		std::cerr << "The file '" + getDBFile() + "' is empty" << '\n';
		return false;
	}

	/* parse the file we read */
	try {
		Wt::Json::Object result;
		Wt::Json::parse(Wt::WString::fromUTF8(file).toUTF8(), result);

		/* parse the comments we read */
		const Wt::Json::Array& jsonComments = readJSONValue<Wt::Json::Array>(result, "comments", Wt::Json::Array());
		for (size_t i = 0; i < jsonComments.size(); i++)
			comments.push_back(readJSonComment(jsonComments[i]));

		/* parse the unsubscribers we read */
		const Wt::Json::Array& jsonUnsub = readJSONValue<Wt::Json::Array>(result, "unsubscribers", Wt::Json::Array());
		for (size_t i = 0; i < jsonUnsub.size(); i++)
			unsubscribers.push_back(readJSonUnsub(jsonUnsub[i]));
	}
	catch (Wt::WException error)
	{
		std::cerr << "Error while parsing file '" << getDBFile() << "': " << error.what() << "\n" << std::endl;

		/* email */
		Wt::WString msg, title = "Error while parsing thread '{1}':";
		title = title.arg(client.thread);
		msg = "{1}\n\nError = '{2}'\n\nFile = '{3}'";
		msg.arg(title).arg(error.what()).arg(file);
		sendEmail.send(title, msg, SendEmail::PLAIN);

		/* save the file */
		std::ofstream of((getDBFile()+ "_" + Wt::WDate::currentDate().toString() + "_.sav").toUTF8().c_str());
		of << file;
		of.close();
	}

	return true;
}

bool CommentsDB::saveFile(std::vector<Comment> &comments, std::vector<std::string> &unsubscribers)
{
	/* write the comments to the file */
	std::ofstream db(getDBFile().c_str());
	if (db.is_open())
	{
		db << "{\n";

		db << "\t\"unsubscribers\":\n";
		db << "\t[\n";
		size_t unsub_size = unsubscribers.size();
		for (size_t i = 0; i < unsub_size; ++i)
		{
			if (i > 0) {
				db << ",\n";
			}
			/* replace the " character by it's html equivalent */
			std::string email = encodeJSONString(unsubscribers[i]);
			db << "\t\t{ \"email\": \"" << email << "\" }";
		}
		db << "\n\t],\n\n";

		db << "\t\"comments\":\n";
		db << "\t[\n";
		for (size_t i = 0; i < comments.size(); i++)
		{
			std::string author = comments[i].author().toUTF8();
			std::string email = comments[i].email().toUTF8();
			std::string msg = comments[i].msg().toUTF8();

			/* replace the " character by it's html equivalent */
			author = encodeJSONString(author);
			email = encodeJSONString(email);
			msg = encodeJSONString(msg);

			db << "\t\t{ \"author\": \"" << author;
			db << "\", \"email\": \"" << email;
			db << "\", \"date\": " << comments[i].date().toJulianDay();
			db << ", \"time\": \"" << comments[i].time().toString();
			db << "\", \"msg\": \"" << msg;
			db << "\", \"IP\": \"" << comments[i].clientAddress();
			db << "\", \"sessionId\": \"" << comments[i].sessionId();

			if (i < comments.size() - 1)
				db << "\" },\n\n\n";
			else
				db << "\" }\n";
		}

		db << "\t]\n";
		db << "}\n";

		/* TODO: find a way to fsync! */
		db.close();

		return true;
	} else
		return false;
}

std::vector<std::string> CommentsDB::emailSubscribers()
{
	std::vector<Comment> comments;
	std::vector<std::string> unsubs;

	if (!parseFile(comments, unsubs))
		return std::vector<std::string>();

	std::set<std::string> subs;
	for (size_t i = 0; i < comments.size(); i++)
		subs.insert(comments[i].email().toUTF8());

	/* get rid of invalid emails */
	subs.erase("");

	for (size_t i = 0; i < unsubs.size(); i++)
		subs.erase(unsubs[i]);

	return std::vector<std::string>(subs.begin(), subs.end());
}

bool CommentsDB::checkUnsubscribers(std::vector<Comment> &comments,
                                    std::vector<std::string> &unsubscribers,
                                    const std::string &email,
                                    Wt::WString &error)
{
	std::set<std::string> subs;
	for (size_t i = 0; i < comments.size(); i++)
		subs.insert(comments[i].email().toUTF8());

	if (!subs.erase(email)) {
		error = "The email '" + email + "' was not subscribed";
		return false;
	}

	subs.clear();
	for (size_t i = 0; i < unsubscribers.size(); i++)
		subs.insert(unsubscribers[i]);

	if (subs.erase(email)) {
		error = "The email '" + email + "' was already unsubscribed";
		return false;
	}

	return true;
}

void CommentsDB::saveNewComment(const Comment &comment)
{
	std::vector<Comment> comments;
	std::vector<std::string> unsubs;

	boost::recursive_mutex::scoped_lock lock_comments(comments_mutex);

	/* read the current comments */
	parseFile(comments, unsubs);

	/* add the new comment */
	comments.push_back(comment);

	/* save the new file */
	saveFile(comments, unsubs);
}

bool CommentsDB::validateComment(const Comment &comment,
                                 Wt::WString &error) const
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

CommentsDB::CommentsDB(Wt::WServer &server, const Wt::WString &url,
                       NewCommentCallback cb) : _server(server)
{
	std::vector<Comment> comments;
	std::vector<std::string> unsubs;
	boost::recursive_mutex::scoped_lock lock(thread_clients_mutex);

	client.sessionID = Wt::WApplication::instance()->sessionId();
	client.cb = cb;
	client.thread = url;

	url_clients[client.thread].push_back(client);

	if (!parseFile(comments, unsubs))
		return;

	/* display the comments */
	for (size_t i = 0; i < comments.size(); i++)
		client.cb(comments[i]);
}

CommentsDB::~CommentsDB()
{
	boost::recursive_mutex::scoped_lock lock(thread_clients_mutex);

	/* erase the client from the client DB */
	std::vector<Client> clients = url_clients[client.thread];
	for (size_t i = 0; i < clients.size(); i++) {
		if (client.sessionID == clients[i].sessionID) {
			url_clients[client.thread].erase(url_clients[client.thread].begin() + i);
			return;
		}
	}
}

bool CommentsDB::postComment(const Comment &comment, Wt::WString &error)
{
	const Wt::WEnvironment& env = Wt::WApplication::instance()->environment();
	boost::recursive_mutex::scoped_lock lock_thread(thread_clients_mutex);
	boost::recursive_mutex::scoped_lock lock_comments(comments_mutex);

	if (!validateComment(comment, error))
		return false;

	/* add the comment to the DB */
	saveNewComment(comment);

	/* warn the other clients that there is a new comment */
	std::vector<Client> clients = url_clients[client.thread];

	for (size_t i = 0; i < clients.size(); i++) {
		if (client.sessionID == clients[i].sessionID)
			clients[i].cb(comment);
		else
			_server.post(clients[i].sessionID, boost::bind(clients[i].cb, comment));
	}

	Config* conf = Config::getConfig();

	/* email */
	Wt::WString url = env.urlScheme() + "://";
	if (conf->websiteURL() == "") {
		url += env.hostName();
	} else {
		url += conf->websiteURL();
	}
	url += env.deploymentPath() + "?url=" + client.thread;
	Wt::WString url_unsub = url + "&unsub=1";
	Wt::WString msg = "<p>Hi " + conf->websiteName() + " users!</p>" \
			  "<p>There is a new comment from '{1}' on article <a href=\"{2}\">{2}</a>:</p>" \
			  "<p>If you don't want to receive mails from the notification list anymore, " \
			  "please <a href=\"{3}\">unsubscribe</a>.</p>" \
			  "<p>------------------------------</p>{4}";
	msg = msg.arg(comment.author()).arg(url).arg(url_unsub).arg(comment.msg());
	sendEmail.send("[" + conf->websiteName() + "] New comment at " + client.thread, msg, SendEmail::HTML, emailSubscribers(), true);

	return true;
}

bool CommentsDB::unsubscribe(const std::string &email, Wt::WString &error)
{
	std::vector<Comment> comments;
	std::vector<std::string> unsubs;

	boost::recursive_mutex::scoped_lock lock_thread(thread_clients_mutex);
	boost::recursive_mutex::scoped_lock lock_comments(comments_mutex);

	/* read the current comments */
	if (!parseFile(comments, unsubs))
		return false;

	/* add the new unsubscriber */
	if (checkUnsubscribers(comments, unsubs, email, error))
		unsubs.push_back(email);
	else
		return false;

	/* save the new file */
	if (!saveFile(comments, unsubs))
		return false;

	Config* conf = Config::getConfig();

	/* email */
	Wt::WString msg = "<p>Hi " + conf->websiteName() + " user!</p>" \
			"<p>Your unsubscription has been taken into account on thread '{1}'</p>";
	msg = msg.arg(client.thread);

	std::vector<std::string> to;
	to.push_back(email);

	sendEmail.send("[" + conf->websiteName() + "] Unsubscribing to thread "
		       + client.thread, msg, SendEmail::HTML, to, false);

	return true;
}
