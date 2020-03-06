#ifndef CHAT_COMMON_H
#define CHAT_COMMON_H

#include<iostream>
#include<list>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<string>

//default server ip
#define SERVER_IP "127.0.0.1"

//default server port
#define SERVER_PORT 1234

//int epoll_create(int EPOLL_SIZE)
#define EPOLL_SIZE 5000

//BUFFER_SIZE
#define BUF_SIZE 65535

//Welcome message for new client
#define SERVER_WELCOME "Welcome to the Chat room!"

//Prefix for client 2 client chatting
#define SERVER_MESSAGE "ClientID: %d say>>%s"

//Send Notice when there is the only client.
#define CAUTION "There is only one client in this chat room!"

//Link new fd to epollfd.
//"bool enable_et" fot mode true--on(ET),false--off(LT)
static void addfd(int epollfd,int fd,bool enable_et){
	struct epoll_event ev;
	ev.data.fd=fd;
	ev.events=EPOLLIN;
	if(enable_et)
		ev.events=EPOLLIN | EPOLLET;//recv IN message and set ET mode.
	epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);//can be (.., ev.data.fd,&ev)

	fcntl(fd,F_SETFL,fcntl(fd,F_GETFD,0)|O_NONBLOCK);
	/*对FD进行F_SETFL操作，操作符是后面的
	 * fcntl(取close on exec)和nonblock的位或| ，
	 * 意思是同时执行这两个操作:根据close on exec
	 * 防止子进程越权、设置fd为非阻塞*/
	printf("fd added to epoll!\n");
}

#endif //CHAT_COMMON_H
