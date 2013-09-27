#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>

using namespace std;
int main(int argc, char* argv[]){
  int nThreads = 3;
  int port = atoi( argv[1]);
    struct sockaddr_in serv_addr;
	if(argc>2)
	nThreads = atoi (argv[2]);
//time to prepare the socket
  int theSocket = socket(PF_INET,SOCK_STREAM,0);
	serv_addr.sin_family = PF_INET;
	serv_addr.sin_port = port;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	//sets the server struct to the port and server type.
	if(theSocket<0){
		cout<<"something went wrong with the socket.";
		exit(1);
	}
  //int connection = connect(theSocket,(struct sockaddr *)&port,0);
  //if(connection<0)
   // cout<<"connection failed.";
int connection;
while(true){//need to keep looking for a connection
	connection = connect(theSocket,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
	if(!connection)
		break;//if we get a connection, break
}

}
