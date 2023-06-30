#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 8080

using namespace std;

int main(int argc, char* argv[]){
	int status, valread, client_fd;
	int sockfd = socket(AF_INET,SOCK_STREAM, 0);
	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<= 0){
        	printf("\nInvalid address/ Address not supported \n");
        	return -1;
	}
	connect(client_fd,(sockaddr*)&serv_addr,sizeof(serv_addr));
	char message[]="working";
	write(client_fd,message,sizeof(message));
	return 0;
}
