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
#include <limits.h>

boost::recursive_mutex CommentsDB::thread_clients_mutex;
std::map<Wt::WString, std::vector<CommentsDB::Client> > CommentsDB::thread_clients;

boost::recursive_mutex CommentsDB::comments_mutex;

/* WARNING: This function is not portable! Linux ONLY! */
std::string CommentsDB::getDBDirectory() const
{
	char path[PATH_MAX];
	size_t len = readlink("/proc/self/exe", path, PATH_MAX);
	path[len] = '\0';

	std::string dbPath(path);
	std::size_t dir = dbPath.find_last_of('/');
	if (dir != std::string::npos)
		return dbPath.substr(0, dir) + "/db/";
	else
		return "./db/";
}

std::string CommentsDB::getDBFile() const
{
	return getDBDirectory() + client.thread.toUTF8() + ".json";
}

Comment CommentsDB::readJSonComment(const Wt::Json::Object &object)
{
	const Wt::WString &author = object.get("author");
	Wt::WDate date = Wt::WDate::fromJulianDay((int)object.get("date"));
	const Wt::WString &msg = object.get("msg");

	return Comment(author, msg, date);
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

	/* parse the file we read */
	Wt::Json::Object result;
	Wt::Json::parse(file, result);
	const Wt::Json::Array& jsonComments = result.get("comments");

	/* parse the comments we read */
	for (size_t i = 0; i < jsonComments.size(); i++)
		comments.push_back(readJSonComment(jsonComments[i]));

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
			std::string msg = comments[i].msg().toUTF8();

			/* replace the " character by it's html equivalent */
			strReplace(author, "\"", "&#34;");
			strReplace(msg, "\"", "&#34;");

			db << "		{ \"author\": \"" << author;
			db << "\", \"date\": " << comments[i].date().toJulianDay();
			db << ", \"msg\": \"" << msg;
			db << "\" }" << std::endl << std::endl << std::endl;
		}

		db << "	]" << std::endl;
		db << "}" << std::endl;

		/* TODO: find a way to fsync! */
		db.close();
	}
}

CommentsDB::CommentsDB(Wt::WServer &server, const Wt::WString &thread, NewCommentCallback cb)
		: _server(server)
{
	boost::recursive_mutex::scoped_lock lock(thread_clients_mutex);

	client.sessionID = Wt::WApplication::instance()->sessionId();
	client.cb = cb;
	client.thread = thread;

	thread_clients[client.thread].push_back(client);

	/* display the comments */
	std::vector<Comment> comments = readCommentsFromFile();
	for (size_t i = 0; i < comments.size(); i++)
		client.cb(comments[i]);
}

CommentsDB::~CommentsDB()
{
	boost::recursive_mutex::scoped_lock lock(thread_clients_mutex);

	/* erase the client from the client DB */
	std::vector<Client> clients = thread_clients[client.thread];
	for (size_t i = 0; i < clients.size(); i++) {
		if (client.sessionID == clients[i].sessionID) {
			thread_clients[client.thread].erase(thread_clients[client.thread].begin() + i);
			return;
		}
	}
}

void CommentsDB::postComment(const Comment &comment)
{
	boost::recursive_mutex::scoped_lock lock_thread(thread_clients_mutex);
	boost::recursive_mutex::scoped_lock lock_comments(comments_mutex);

	/* add the comment to the DB */
	saveNewComment(comment);

	/* warn the other clients that there is a new comment */
	std::vector<Client> clients = thread_clients[client.thread];

	for (size_t i = 0; i < clients.size(); i++) {
		if (client.sessionID == clients[i].sessionID)
			clients[i].cb(comment);
		else
			_server.post(clients[i].sessionID, boost::bind(clients[i].cb, comment));
	}
}
