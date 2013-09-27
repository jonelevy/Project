

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>



#define BACKLOG 10

void sigchld_handler(int s){
	while(waitpid(-1,NULL,WNOHANG) > 0);
}
//get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc,char** argv){
int sockfd,new_fd;//for new connection and listening
struct addrinfo hints,*servinfo,*p;
struct sockaddr_storage their_addr;
socklen_t sin_size;
struct sigaction sa;
int yes=1;//boolean style int
char s[INET6_ADDRSTRLEN];
int rv;

int portNo;
char* extensions[6]={".html",".pdf",".jpeg",".gif",".png"};
char* path;
if (argc<3)
	printf("We need a port number and default directory. the commandline should look like (./hw2 <portnumber> <path>");
portNo=atoi(argv[1]);
path=argv[2];

//printf("server starting on %i with directory %s \n",portNo,path);

int clients[1024];//for connected clients
int a,b,c;//index ints
for(a=0;a<1024;a++){
	clients[a]=0;

}
int slot=0;//just another int. used for client side listening
//start up the server
memset (&hints, 0, sizeof(hints));//set some memory for the server
hints.ai_family=AF_INET;
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;

if(getaddrinfo(NULL, argv[1],&hints, &servinfo) !=0){
	fprintf(stderr, "getaddrinfo: %s/n",gai_strerror(rv));
	return 1;
}
//binding
printf("binding server \n");
for(p=servinfo;p!=NULL;p=p->ai_next){
	if((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1){
		perror("server:socket");
		continue;
	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
		perror("setsockopt");
		exit(1);
	}

	if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
		close(sockfd);
		perror("server:bind");
		continue;
	}
	break;
}
if(p==NULL){
	fprintf(stderr, "server: failed to bind\n");
	return 2;
}
freeaddrinfo(servinfo);
printf("preparing to listen \n");


//listen for available spots
if(listen (sockfd,1000000) !=0){
	perror("listen error \n");
	exit(1);
}


//printf("server open \n");
while(1)
{
//	printf("loop sequence \n");
	sin_size=sizeof(their_addr);
	printf("size is %d\n",sin_size);
	clients[slot] = accept(sockfd, (struct sockaddr*) &their_addr,&sin_size);
	printf("client accept \n");
	if(clients[slot]<0)
		error("We have a problem");
	else{
		if(fork()==0){
			//client connection
			printf("client connected \n");
			//time for new variables to initiate client side actions
			char mesg[99999],*reqline[3],data_to_send[1024],path[99999];
			int rcvd,fd, bytes_read;
			
			memset ((void*)mesg, (int)'\0',99999);//make sure everything is set for very large numbers
			rcvd = recv(clients[slot],mesg,99999,0);

			if(rcvd<0)//theres clearly a problem
				fprintf(stderr,"error recieving \n");
		
			else if(rcvd==0)//the socket closed. Bad socket, don't do that!
				fprintf(stderr,"The socket closed on us! That's not nice!");

			else{//things are working
				printf("%s",mesg);


				reqline[0]=strtok (mesg," \t\n");
				if(strncmp(reqline[0], "GET\0", 4)==0){
					reqline[1]= strtok (NULL, " \t");
					reqline[2]=strtok (NULL, " \t\n");
					if( strncmp(reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1",8)!=0){
						write(clients[slot], "HTTP/1.0 400 Bad Request \n",25);
					}
					else{
						if (strncmp(reqline[1], "/\0", 2)==0)
							reqline[1] = "/index.html";//if no file is specified
					
						if((fd = open(path,O_RDONLY))!=-1){//We got us a file!
							send(clients[slot], "HTTP/1.0 200 OK\n\n", 17, 0);
							while( (bytes_read=read(fd,data_to_send,1024))>0)
								write(clients[slot],data_to_send, bytes_read);
						}
						else
							write(clients[slot], "HTTP/1.0 404 Not Found\n", 23);//No file here.
					}
				}
			}


			shutdown (clients[slot], SHUT_RDWR);
			close(clients[slot]);
			clients[slot]=-1;

			exit(0);
		}
	}
	while (clients[slot]>=0) slot=(slot+1)%1000;
}
	return 0;
//printf("end");
}
