#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include "common.h"
using namespace std;

class Server{
public:
	Server();

	void Removeclientfd(int& clientfd);
	void Init();
	void Close();
	void Start();
private:
	int Broadcastmsg(int clientfd);

	int listener; //listen sock
	struct sockaddr_in serverAddr;

	int epfd;

	list<int>clients_list;
};
#endif//CHAT_SERVER_H
