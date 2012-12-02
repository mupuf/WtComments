#ifndef COMMENTSDB_H
#define COMMENTSDB_H

#include <vector>
#include <map>
#include <boost/function.hpp>
#include <boost/thread.hpp>

#include "comment.h"
#include "sendemail.h"

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
		Wt::WString url;
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

	Comment readJSonComment(const Wt::Json::Object &object);
	std::vector<Comment> readCommentsFromFile();
	void saveNewComment(const Comment &comment);

	bool validateComment(const Comment &comment, Wt::WString &error) const;

public:
	CommentsDB(Wt::WServer &server, const Wt::WString &url, NewCommentCallback cb);
	~CommentsDB();
	bool postComment(const Comment &comment, Wt::WString &error);
};

#endif // COMMENTSDB_H
