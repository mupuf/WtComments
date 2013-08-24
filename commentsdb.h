#ifndef WTCOMMENTS_COMMENTSDB_H
#define WTCOMMENTS_COMMENTSDB_H

#include "comment.h"
#include "sendemail.h"

#include <boost/function.hpp>
#include <boost/thread.hpp>

#include <map>
#include <vector>

class View;

namespace Wt { class WServer; }

typedef boost::function<void (const Comment&)> NewCommentCallback;

class CommentsDB : boost::noncopyable
{
private:
	Wt::WServer &_server;
	struct Client
	{
		std::string sessionID;
		NewCommentCallback cb;
		Wt::WString full_url;
		Wt::WString thread;
	} client;
	SendEmail sendEmail;

	std::string encodeJSONString(std::string str);
	std::string decodeJSONString(std::string str);

	/* associate clients to a comment thread */
	static boost::recursive_mutex thread_clients_mutex;
	static std::map<Wt::WString, std::vector<Client> > url_clients;

	static boost::recursive_mutex comments_mutex;

	std::string getDBDirectory() const;
	std::string getDBFile() const;

	bool checkUnsubscribers(std::vector<Comment> &comments,
				std::vector<std::string> &unsubscribers,
				const std::string &email, Wt::WString &error);

	Comment readJSonComment(const Wt::Json::Object &object);
	std::string readJSonUnsub(const Wt::Json::Object &object);
	bool parseFile(std::vector<Comment> &comments, std::vector<std::string> &unsubscribers);
	bool saveFile(std::vector<Comment> &comments, std::vector<std::string> &unsubscribers);
	void saveNewComment(const Comment &comment);

	std::vector<std::string> emailSubscribers();

	bool validateComment(const Comment &comment, Wt::WString &error) const;

public:
	CommentsDB(Wt::WServer &server, const Wt::WString &url, NewCommentCallback cb);
	~CommentsDB();
	bool postComment(const Comment &comment, Wt::WString &error);
	bool unsubscribe(const std::string &email, Wt::WString &error);
};

#endif // WTCOMMENTS_COMMENTSDB_H
