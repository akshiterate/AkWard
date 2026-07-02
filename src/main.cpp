#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <cstddef>
#include <netdb.h>
#include <string>
#include <fstream>
#include <unistd.h>

//definitions
#define BACKLOG 10


int setup_server(){
	int sockfd;
	struct addrinfo hints,*res;

	memset(&hints, 0,sizeof(hints));//this function sets every byte in hints to 0
	hints.ai_family = AF_UNSPEC;//can be ipv4(AF_INET) or ipv6(AF_INET6)
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;//fill in my IP

	int status = getaddrinfo(NULL,"8080",&hints,&res);//prepares an address struct making res point to ai_family, ai_socktype,etc
	if(status != 0){
		std::cerr<<gai_strerror(status); //cerr writes to standard error( outputs to the terminal but makes it easy to differentiate between a log and an error
						 //but getaddrinfo uses its own error code so cerr<<gaistrerror converts error code into a readable msg
		exit(1);
	}

	sockfd = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	//'->' is being used because remember res is a struct
	if(sockfd == -1){
		perror("socket");// looks at errno(OS stores failure reason here) prints a human readable version of it.
		exit(1);
	}
	
	if(bind(sockfd,res->ai_addr,res->ai_addrlen) == -1){
		perror("bind");
		exit(1);
	}

	if(listen(sockfd,BACKLOG) == -1){
		perror("listen");
		exit(1);
	
	}
	freeaddrinfo(res);
	return sockfd;
}


std::string build_response(char *req){
	char *first_space = strchr(req,' ');//find the 1st ' ' in req
	char * sec_space = strchr(first_space+1,' '); // since strchr works with pointers req is just the 1st element(char) and if we give first_space now the char array starts from there
	int plen = sec_space-first_space-1;
	char path[256];
	memcpy(path,first_space+1,plen);
	path[plen] = '\0';
	std::string filepath;
	if(strcmp(path,"/")==0){
		filepath = "../webpages/index.html";
	}else filepath = "../webpages"+std::string(path);
	std::ifstream file(filepath);
	bool found = true;
	std::string line,contents;
	if(file){
		while(getline(file,line)){
			contents+=line;
			contents+='\n';
		}
		
	}else {
		found = false;
		filepath = "../webpages/404.html";
		std::ifstream file(filepath);
		while(getline(file,line)){
			contents+=line;
			contents+='\n';
		}
	}
	std::string msg;
	if(found) msg = "HTTP/1.0 200 OK \r\n\r\n"+contents;
	else{
		msg = "HTTP/1.0 404 Not Found\r\n\r\n"+contents;
	}
	return msg;
}
//recieving functions
char* filled(char *req,char *buf,int &cap,int pos,int bpos){
	if(pos+bpos>cap){
		cap *=2;
		char* temp = (char*)realloc(req,cap);//never assume realloc works
		if(temp == nullptr){
			free(req);
			return nullptr;
		}
		req = temp;
	}
	return req;
}
bool end(char *req,int pos){
	if(pos<4) return false;
	if(req[pos-1] == '\n' && req[pos-2] == '\r' && req[pos-3] == '\n' && req[pos-4] == '\r') return true;
	return false;
}
char *recieve_req(int client){
	int buf_cap = 1024;
	char buf[buf_cap];
	int req_cap = 4096;
	char *req = (char*)malloc(req_cap);
	int pos = 0;
	while(end(req,pos)==false){
		int bpos = recv(client,buf,buf_cap,0);
		if(bpos < 0){
			perror("recv");
			free(req);
			return nullptr;
		}
		else if(bpos == 0) return nullptr;
		req = filled(req,buf,req_cap,pos,bpos);
		memcpy(&req[pos],buf,bpos);
		pos+=bpos;
	}
	return req;
}


int main(){
	int sockfd = setup_server();
	while(true){
		struct sockaddr_storage their_addr;//sockaddr_storage is where the client info is stored
		socklen_t addr_size = sizeof(their_addr);

		int newfd = accept(sockfd,(struct sockaddr*)&their_addr,&addr_size);
		if(newfd == -1){
			perror("accept");
			continue;
		}
		char *req = recieve_req(newfd);
		std::string msg = build_response(req);
		free(req);
		int len = msg.size();
		int n = send(newfd,msg.c_str(),len,0);
		close(newfd);
	}
	return 0;


}
