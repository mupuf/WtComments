#include "commentsdb.h"

#include "view.h"

#include <Wt/WServer>
#include <Wt/WApplication>

boost::recursive_mutex CommentsDB::thread_clients_mutex;
std::map<Wt::WString, std::vector<CommentsDB::Client> > CommentsDB::thread_clients;

CommentsDB::CommentsDB(Wt::WServer &server, const Wt::WString &thread, NewCommentCallback cb)
		: _server(server)
{
	boost::recursive_mutex::scoped_lock lock(thread_clients_mutex);

	client.sessionID = Wt::WApplication::instance()->sessionId();
	client.cb = cb;
	client.thread = thread;

	thread_clients[client.thread].push_back(client);
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
	boost::recursive_mutex::scoped_lock lock(thread_clients_mutex);

	std::vector<Client> clients = thread_clients[client.thread];

	for (size_t i = 0; i < clients.size(); i++) {
		if (client.sessionID == clients[i].sessionID)
			clients[i].cb(comment);
		else
			_server.post(clients[i].sessionID, boost::bind(clients[i].cb, comment));
	}


}
