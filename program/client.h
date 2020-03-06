#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include"common.h"

// class CLIENT
class Client{
public:
	Client();
	void Connect();
	void Close();
	void Start();
private:
	int sock;
	int pid;
	int epfd;
	bool isClientwork;
	int pipe_fd[2];
	char message[BUF_SIZE];
	struct sockaddr_in serverAddr;
};

#endif //CHAT_CLIENT_H
