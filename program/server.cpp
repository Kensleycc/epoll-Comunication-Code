#include "server.h"
using namespace std;

Server::Server(){
	serverAddr.sin_family=PF_INET;
	serverAddr.sin_port=htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr=inet_addr(SERVER_IP);
	listener=0;
	epfd=0;
}

void Server::Init(){
	
	cout<<"Init Server......"<<endl;
	listener=socket(PF_INET,SOCK_STREAM,0);
	if(listener<0){
		perror("sock listener error:");exit(-1);
	}

	/*int on=1;  
    if((setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
    {  
        perror("setsockopt failed");  
        exit(EXIT_FAILURE);  
    }  */	
	
	if(bind(listener,(struct sockaddr*)&serverAddr,sizeof(serverAddr))<0){
		perror("bind error:");exit(-1);
	}

	if(listen(listener,5)){
		perror("listen error:");exit(-1);
	}
	cout<<"Start listening:"<<SERVER_IP<<':'<<SERVER_PORT<<endl;
			
	epfd=epoll_create(EPOLL_SIZE);
	if(epfd<0){
		perror("epfd error:");exit(-1);
	}

	addfd(epfd,listener,true); //Keystep.Add listener to epoll and set NONBLOCK.
	cout<<"Succeed Initting Server."<<endl;
}

void Server::Close(){	//close socket gentlely.
	close(listener);
	close(epfd);
}

void Server::Removeclientfd(int& clientfd){
	close(clientfd);

	clients_list.remove(clientfd);
	cout<<clientfd<<" closed.Now there are "
		<<clients_list.size()<<" clients left."<<endl;
}

int Server::Broadcastmsg(int clientfd){
	char buf[BUF_SIZE],message[BUF_SIZE];
	//save message from client in buf[].
	//save msg in message[],which has been combined.
	memset(buf,0,BUF_SIZE);
	memset(message,0,BUF_SIZE);

	cout<<"(recv from client:"<<clientfd<<"):";
	int len =recv(clientfd,buf,BUF_SIZE,0);

	if(!len){//len=0. client closed.
		
		cout<<"Client:"<<clientfd<<"closed"<<endl;
		Removeclientfd(clientfd);
	}else{	//len!=0. msg from clientfd.

		if(clients_list.size()==1){	//there is solely clientfd.		
			send(clientfd,CAUTION,strlen(CAUTION),0);
			return len;
		}
		sprintf(message,SERVER_MESSAGE,clientfd,buf);


		for(list<int>::iterator it=clients_list.begin();
			it!=clients_list.end();++it){
			//send message to every client in list, except clientfd.
		
			if(*it!=clientfd)
				if(send(*it,message,BUF_SIZE,0)<0){
					cout<<"send to:"<<*it<<"error."<<endl;
					continue;
				}			
		}
	}
	return len;
}

void Server::Start(){
	//Epoll event quene.
	static struct epoll_event events[EPOLL_SIZE];

	Init();//initialization} Server.

	while(1){
		int nfds=epoll_wait(epfd,events,EPOLL_SIZE,-1);

		if(nfds<0){
			perror("epoll failure:");Close();exit(-1);
		}

		cout<<"epoll calls.nfds= "<<nfds<<endl<<endl;

		for(int i=0;i<nfds;++i){	//process epoll calls.
			int sockfd=events[i].data.fd;

			if(sockfd==listener){//New client connection to server.
				
				//do accepting.
				struct sockaddr_in clientAddr;
				socklen_t clientAddrlen=sizeof(sockaddr);
				int clientfd=accept(sockfd,(sockaddr*)&clientAddr,&clientAddrlen);
				cout<<"Connection from:"<<inet_ntoa(clientAddr.sin_addr)
					<<':'<<ntohs(clientAddr.sin_port)<<" success."<<endl;
				
				//add clientfd into epollfd and list.	
				addfd(epfd,clientfd,true);
				clients_list.push_back(clientfd);
				cout<<"Add new client to epoll and list."<<endl
					<<"Now there are "<<clients_list.size()<<" clients in chatroom."
					<<endl;

				//send Welcom.
				int ret=send(clientfd,SERVER_WELCOME,strlen(SERVER_WELCOME),0);
				if(ret<0){
					perror("welcome error:");Close();exit(-1);
				}else cout<<"Welcomed "<<clientfd<<endl;
			}//if (connection from new clients)
			
			else{//msg being broadcasted from clients 
				int ret=Broadcastmsg(sockfd);
				if(ret<0){
					perror("broadcast error:");Close();exit(-1);
				}
				
			}//else(msg broadcast)
	
		}//for nfds
	}//while
	Close();
}//Start()
