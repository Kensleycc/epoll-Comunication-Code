#include"client.h"

Client::Client():
	sock(0),
	pid(0),
	epfd(0),
	isClientwork(true){
	serverAddr.sin_family=PF_INET;
	serverAddr.sin_port=htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr=inet_addr(SERVER_IP);
}

void Client::Connect(){
	std::cout<<"Connect Serv:"<<SERVER_IP<<":"<<SERVER_PORT<<std::endl;

	//new socket
	sock=socket(PF_INET,SOCK_STREAM,0);
	if(sock<0){
		perror("socket error:");exit(-1);
	}
	//do connect
	if(connect(sock,(struct sockaddr*)&serverAddr,sizeof(serverAddr))<0){
		perror("connect error:");exit(-1);
	};
	//new pipe for Inter Process Comunication.
	if(pipe(pipe_fd)<0){
		perror("pipe error:");exit(-1);
	}
	//new epoll_fd.
	epfd=epoll_create(EPOLL_SIZE);
	if(epfd<0){
		perror("epfd error:");exit(-1);
	}
	//add sock and read pipe to epfd.
	addfd(epfd,sock,true);
	addfd(epfd,pipe_fd[0],true);
}

void Client::Close(){
	close(sock);
	//parents process
	if(pid) close(pipe_fd[0]);
	else //son process 
		close (pipe_fd[1]);
}

void Client::Start(){
	Connect();

	static struct epoll_event events[2];//for sock and pipe[0]
	pid=fork();

	if(pid<0){
		perror("fork error:");
		close(sock);
		exit(-1);
	}else if(pid==0){ //SON process(get cin from stdin and send to parent)
		close(pipe_fd[0]);//turn off the read side.

		std::cout<<"Type \"EXIT\" to leave!"<<std::endl;

		//when working,get message from client instantly.
		while(isClientwork){
			memset(message,0,BUF_SIZE);
			fgets(message,BUF_SIZE,stdin);//msg:****\n\0
			size_t msglen=strlen(message);
		
			//when get "Exit"
			if(strncmp(message,"EXIT",4)==0&&msglen==5){
				isClientwork =0;
			}else{//send msg into pipe				
				if(write(pipe_fd[1],message,msglen)<0){
					perror("write error:");exit(-1);
				}
			}
		
		}
	}else{//parents process
		close(pipe_fd[1]);	
		
		while(isClientwork){
			int nfds=epoll_wait(epfd,events,2,-1);

			for(int i(0);i<nfds;++i){
				memset(message,0,BUF_SIZE);				//Msg from server.
				if(events[i].data.fd==sock){					
					int ret =recv(sock,message,BUF_SIZE,0);					 
					if (!ret) {//ret=0 e.t.server close.						
						std::cout<<"Connection closed:"<<sock<<std::endl;						
						close(sock);
						isClientwork=0;
					}else	std::cout<<message<<std::endl;
				}else{
					//Msg from SON process.
					int ret =read(events[i].data.fd,message,BUF_SIZE);	
					if(ret==0)	isClientwork=false;  //client type 'exit'
					else send(sock,message,BUF_SIZE,0);
				}
			}//for
		}//while		
	}//parents process
	Close();
}
