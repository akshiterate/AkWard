#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <cstddef>
#include <netdb.h>
#include <string>
#include <fstream>
#include <unistd.h>
#include <ctime>

//definitions
#define BACKLOG 10
#define MAX_REQ_SIZE 8192

void server_log(std::string msg);
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

//reponse building
bool ends_with(const std::string &str,const std::string &suf){
	if(str.size()<suf.size()) return false;
	return str.compare(str.size()-suf.size(),suf.size(),suf)==0;
}
std::string get_content_type(std::string &filepath){//new c++ feature discovered: you can reference variables using '&' when its used in a function
	if(ends_with(filepath,".html")) return "text/html";
	if(ends_with(filepath,".css")) return "text/css";
	if(ends_with(filepath,".png")) return "image/png";
	if(ends_with(filepath,".jpeg")) return "image/jpg";
	if(ends_with(filepath,".gif")) return "image/gif";
	return "application/octet-stream";
}
std::string build_response(char *req){
	if(strncmp(req,"GET ",4)!=0){
		return "HTTP/1.0 405 Method Not Allowed\r\n\r\n";
	}
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
	server_log("GET "+filepath);
	std::ifstream file(filepath,std::ios::binary);//we need to open the file in binary mode, this prevents conversion of images to text which allows the support of images on the server
	bool found = true;
	bool bad = false;
	std::string contents;
	if(file){
		if(strstr(path,"..")){
			bad = true;
			filepath = "../webpages/403.html";
			std::ifstream file(filepath,std::ios::binary);
			file.seekg(0,std::ios::end);//move the read pointer to the end of the file
			size_t size = file.tellg();// tellg returns current position of read pointer, since we are at the end we get the size.
			file.seekg(0);//moves read pointer back to 0;
			contents.resize(size);
			file.read(contents.data(),size);// data.data() gives you a pointer to internal memory of a vector/string
		}
		else{
			file.seekg(0,std::ios::end);
			size_t size = file.tellg();
			file.seekg(0);
			contents.resize(size);
			file.read(contents.data(),size);
		}
		
	}else {
		found = false;
		filepath = "../webpages/404.html";
		std::ifstream file(filepath,std::ios::binary);
		file.seekg(0,std::ios::end);
		size_t size = file.tellg();
		file.seekg(0);
		contents.resize(size);
		file.read(contents.data(),size);
	}
	std::string msg;
	std::string content_type = get_content_type(filepath);
	std::string cache_con = "Cache-Control: no-cache, no-store, must-irevalidate\r\nPragma: no-cache\r\nExpires: 0";
	if(bad) msg = "HTTP/1.0 403 Forbidden Content-Type: "+cache_con+content_type+"\r\n\r\n"+contents;
	else if(found) msg = "HTTP/1.0 200 OK\r\n"+cache_con+"\r\nContent-Type: "+content_type+"\r\nContent-Length: "+std::to_string(contents.size())+"\r\n\r\n"+contents;
	else{
		msg = "HTTP/1.0 404 Not Found Content-Type: "+cache_con + content_type+"\r\n\r\n"+contents;
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
		if(pos> MAX_REQ_SIZE){
			free(req);
			return nullptr;
		}
	}
	return req;
}


void server_log(std::string msg){
	std::ofstream logfile("../logs/server.log",std::ios::app);//"std::ios::app" means open the app in append mode i.e. dont re write over the file but append to it
	if(!logfile){
		std::cerr<<"Failed to open log file\n";
		return;
	}
	time_t now = time(nullptr);
	char* time_str = ctime(&now);
	time_str[strlen(time_str)-1] = '\0';
	logfile<<time_str<<": "<<msg<<std::endl;
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
