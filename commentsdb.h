#ifndef COMMENTSDB_H
#define COMMENTSDB_H

#include <vector>
#include <map>
#include <boost/function.hpp>
#include <boost/thread.hpp>

#include "comment.h"

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
		Wt::WString thread;
	} client;

	/* associate clients to a comment thread */
	static boost::recursive_mutex thread_clients_mutex;
	static std::map<Wt::WString, std::vector<Client> > thread_clients;

public:
	CommentsDB(Wt::WServer &server, const Wt::WString &thread, NewCommentCallback cb);
	~CommentsDB();
	void postComment(const Comment &comment);
};

#endif // COMMENTSDB_H
