#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
using namespace std;
int main(int argc, char* argv[]){
//---------setting defaults for vaules-----
int stringLength = 1000;//the default
int searchLength = 0;//the default
int nChildren = 2;
int nThreads = 3;
int blockSize = 4096;
char buffer[256];
struct sockaddr_in serv_addr, cli_addr;
 int port=511337; //default port
ifstream inputFile;
inputFile.open(argv[1]);
//first, if the args are available set new values
 if(argc>=2) 
   stringLength = atoi(argv[2]);
 if(argc>=3)
   searchLength = atoi(argv[3]);
string searchString;
 if(argc>=4)
   nChildren = atoi(argv[4]);
 if(argc>=5)
   nThreads = atoi(argv[5]);
 if(argc>=6)
   blockSize = atoi(argv[6]);
string currentLine;
while(!inputFile.eof()){
getline(inputFile,currentLine);
searchString.append(currentLine);
}
inputFile.close();
//struct sockaddr_in sockStruct;//easier than defining on the spot
int theSocket = socket(PF_INET,SOCK_STREAM,0);//creates the serverside socket
serv_addr.sin_family = AF_INET;
serv_addr.sin_addr.s_addr = INADDR_ANY;
serv_addr.sin_port = port;
 if(bind(theSocket,(struct sockaddr *)&port,0)<0)
   cout<<"failed server creation.";
 listen(theSocket,0);
 int a=0;
   pid_t forkingChildren[nChildren];
char* thePort;
thePort[0]=(char)port;
//listen socket setup before fork
if(listen(theSocket,nChildren+10)<0){

cout<<"failed to listen to client";
exit(1);
}
   for(a=0;a<nChildren;a++){


  forkingChildren[a]=fork();
     	if(forkingChildren[a]<0){//fork failed
			cout<<"something happened and the fork failed. \n";
			exit(1);
		}
	if(forkingChildren[a]==0){//forking is successful
	  if(execlp("searchClient","searchClient",thePort,nThreads,NULL)<0){
	    cout<<"something went wrong";
	    exit(0);
	   }
	}
   


		

   }
 

}
