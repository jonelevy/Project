/* 

Minimal TCP client example. The client connects to port 8080 on the 
local machine and prints up to 255 received characters to the console, 
then exits. To test it, try running a minimal server like this on your
local machine:

echo "Here is a message" | nc -l -6 8080

*/


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#define THEBUFFER 1024
int main(int argc, char** argv) 
{	
	char buffer[THEBUFFER];
	char* extension;
	int sockfd;
  /* inpsired heavily by man 3 getaddrinfo 
   */
if(argc!=2){
fprintf(stderr,"we require a url, or you have more than one. Please fix. \n");
exit(-5);//-5 is failed input
}
int a,b,c;//index ints
const char* startUrl = argv[1];
const char* finalUrl;
char* fileName;
int count=0;
//fprintf(stderr,startUrl);
int theIndex = strlen(argv[1]);
char url[theIndex];
for(a=0;a<theIndex;a++){
	url[a]=0;//zeroing out the array
}
//printf("%d\n",strlen(argv[1]));

for(a=7;a<theIndex;a++){
	//printf(url[count]);
	url[count]=startUrl[a];
	count++;

}
finalUrl=url;
char* path;
char tempPath[20];//so we have a dynamic place to prepare our filename
for(a=0;a<20;a++){
	tempPath[a]=0;//zero out the array
}
int temp;
int slashPosition=0;
if(strstr(finalUrl,"/")!=NULL){
	slashPosition=strchr(finalUrl,'/')-finalUrl;
	path=strstr(finalUrl,"/");
	//fprintf(stderr,path);
	if((strstr(path,".gif")!=NULL)||(strstr(path,".png")!=NULL)||(strstr(path,".html")!=NULL)||(strstr(path,".jpeg")!=NULL)){
		temp=strrchr(path,'/')-path+1;
		count=0;
		//printf("%d\n",temp);
		int thePath = strlen(path);
		for(a=temp;a<thePath;a++){
			//printf("%d\n",a);
			tempPath[count]=path[a];
			count++;
		}
		fileName=tempPath;
		//fprintf(stderr,fileName);

	}
}
else
fileName="index.html";
//printf("%d\n",slashPosition);
//path is everything after that first slash, secondUrl is everything before

char secondUrl[slashPosition];
if(slashPosition>0){
count=0;
for(a=0;a<slashPosition;a++){
	secondUrl[a]=finalUrl[a];
}
for(a=slashPosition+1;a<theIndex;a++){
	path[count]=finalUrl[a];
	count++;
}
finalUrl=secondUrl;
}
//fprintf(stderr,finalUrl);
  struct addrinfo hints;
  struct addrinfo * result, * rp;
  int sock_fd, s;
  hints.ai_socktype = SOCK_STREAM;
  memset(&hints,0,sizeof(struct addrinfo));
  s = getaddrinfo(finalUrl,"80",&hints,&result);
	//printf("%d\n",s);

  if (0 != s){
    perror("error populating address structure");
    exit(1);
  }
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sock_fd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
    if (sock_fd == -1)
      continue;

    if (connect(sock_fd, rp->ai_addr, rp->ai_addrlen) != -1)
	 break; /* Success */

	}
//fprintf(stderr,"test");
const char* slash = "/";
const char* newPath = path;
unsigned char* thePath;
thePath = malloc(strlen(newPath)+1+4);
strcpy(thePath,slash);
strcat(thePath,newPath);
//fprintf(stderr,finalUrl);
//fprintf(stderr,thePath);
sprintf(buffer,"GET %s HTTP/1.0\r\n\r\n",thePath);
//fprintf(stderr,theTest);
send(sock_fd,buffer,strlen(buffer),0);
int bytes_read;
FILE *newFile;
newFile = (FILE*)malloc(30*sizeof(fileName));
newFile=fopen(fileName,"a+");
do{//loop goes through the site and continues to get/print all of it until end of file.
	bzero(buffer,sizeof (buffer));
	bytes_read = recv(sock_fd,buffer, sizeof(buffer),0);
	if(bytes_read>0){
		//fprintf(newFile,"%s",buffer);
		fwrite(buffer,sizeof(buffer[0]),sizeof(buffer)/sizeof(buffer[0]),newFile);
		//printf("%s\n",buffer);
	}
	}while(bytes_read>0);
fclose(newFile);
//fprintf(stderr,finalUrl);

    close(sock_fd);

  if (rp == NULL) {
    fprintf(stderr, "could not connect\n");
    exit(1);
  }

  freeaddrinfo(result);
	shutdown(sock_fd,SHUT_RDWR);
}
