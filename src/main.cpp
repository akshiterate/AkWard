#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <cstddef>
#include <netdb.h>

#define BACKLOG 10

int main(){
	int sockfd;
	struct sockaddr_storage their_addr; //sockaddr_storage is where the client info is stored
	socklen_t addr_size;
	struct addrinfo hints,*res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; //could be ipv4 or ipv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; //fill in my IP
	
	getaddrinfo(NULL,"8080",&hints,&res);

	sockfd = socket(res->ai_family,res->ai_socktype,res->ai_protocol);

	bind(sockfd, res->ai_addr,res->ai_addrlen);
	listen(sockfd,BACKLOG);

	while(true){
		addr_size = sizeof(their_addr);
		int newfd = accept(sockfd,(struct sockaddr*)&their_addr,&addr_size);
		
		char buf[1024];
		int n = recv(newfd,buf,1022,0);
		std::cout<<n<<std::endl;
		buf[1023] = '\0';
		for(int i=0;buf[i]!='\0';i++){
			std::cout<<buf[i];
		}

	}
	return 0;
}
	
