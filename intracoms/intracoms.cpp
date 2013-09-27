\
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
using namespace std;
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED) 
/* union semun is defined by including <sys/sem.h> */ 
#else 
/* according to X/OPEN we have to define it ourselves */ 
union semun { 
      int val;                  /* value for SETVAL */ 
      struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */ 
      unsigned short *array;    /* array for GETALL, SETALL */ 
                                /* Linux specific part: */ 
      struct seminfo *__buf;    /* buffer for IPC_INFO */ 
}; 
#endif 

struct queueContents{
//this makes things a lot easier with the message queues. Keeps everythingi n 1 place.
int childId;int PID;int bufferID;int initialValue;int finalValue;
char mtext[1];
};
struct buffer{
long mtype;
struct queueContents contents;
};//modified verson of the standard msgbuffer struct
int main(int argc, char* argv[]){
cout<<"Jonathan Levy CS ID: jlevy \n";//printing the requirement of thy name
int buffersNo = atoi(argv[1]);//grabs command line arg 1 for the number of buffers

bool semaphoreLock = false;
if(argc>2){
if(strcmp(argv[2],"-lock")==0)
semaphoreLock=true;
}
pid_t theFork[buffersNo/2];//it will fork off buffersNo/2 children when forking happens

int memId = shmget(IPC_PRIVATE,4096,0600|IPC_CREAT);
int * buffers=new int[buffersNo];
int msgQueue = msgget(IPC_PRIVATE,0600 | IPC_PRIVATE);
buffers=(int *)shmat(memId,NULL,0);//preps the memory for children processes
for(int a=0;a<buffersNo/2;a++){
buffers[a]=0;//initialize buffers to 0
}
//int *buffers;
//cout<<"starting fork";
if(semaphoreLock==true){//initialize semaphores here


}
for(int ID=1;ID<=buffersNo/2;ID++){
	int index=ID-1;
	//cout<<ID;cout<<"\n";
	theFork[index] = fork();//ID-1 so that even index 0 is used.
	if(theFork[index]==-1){
		cout<<"issues forking. exiting.";
		exit(-1);
	}
	if(theFork[index]==0){
			
		//cout<<"forking successful";
		*(buffers+index)=ID;//writes off the child IDs
		//size_t byteSize=1024;
		//cout<<*(buffers+index);
		//cout<<*(buffers+index);
		//waitpid(theFork[index],0,-1);
		//wait(0);
		//shmat(memId,buffers,0);
		//cout<<"goodbye! ";cout<<"child id: ";cout<<*(buffers + index); cout <<" PID: 
//";cout<<theFork[index];cout<<"\n";

//---------------------------message queue------------------------------------------------------

		struct buffer message;
		//enter message
		message.mtype=1;
		message.contents.childId=*(buffers+index);
		msgsnd(msgQueue,&message,sizeof(message.contents),0);
		

//------------------------read write cycles------------------------------------------------------
	int bufIndex=0;
	for(int z=0;z<buffersNo;z++){
		
		bufIndex=(buffersNo + ID) % buffersNo;//gives us a buffer index of ID mod buffer index as 
		//stated in projecect overview 4a, and then every buffer/2 child after that stated in 4b
		
		//read 1
		if(semaphoreLock==true){


		}
		int initial = *(buffers + bufIndex);
		int sleepTime = 1/buffersNo;		
		usleep(sleepTime*ID);	
		int final = *(buffers + bufIndex);		
		if(initial!=final){//we have a problem. mtype for error will be 3. we need to look at what the 2 values are, the child id, and the buffer id
		message.mtype=3;
		message.contents.childId = *(buffers+index);
		message.contents.initialValue = initial;
		message.contents.finalValue = final;
		message.contents.bufferID = bufIndex;
		msgsnd(msgQueue,&message,sizeof(message.contents),0);
		}
		
		//read 2
		if(semaphoreLock==true){


		}
		initial = *(buffers + bufIndex);
		sleepTime = 1/buffersNo;		
		usleep(sleepTime*ID);	
		final = *(buffers + bufIndex);		
		if(initial!=final){//we have a problem. mtype for error will be 3. we need to look at what the 2 values are, the child id, and the buffer id
		message.mtype=3;
		message.contents.childId = *(buffers+index);
		message.contents.initialValue = initial;
		message.contents.finalValue = final;
		message.contents.bufferID = bufIndex;
		msgsnd(msgQueue,&message,sizeof(message.contents),0);
		}
		
		//write
		if(semaphoreLock==true){


		}
		initial = *(buffers + bufIndex);
		sleepTime = 1/buffersNo;		
		usleep(sleepTime*ID);
		final = initial + (1 << (ID-1));
		*(buffers+bufIndex)=final;

	}
		//exit message
		message.contents.childId=*(buffers + index);
		message.contents.PID=theFork[index];
		message.contents.bufferID=index;
		//message.contents.mtext="goodbye!";
		message.mtype=2;//our exit message		
		msgsnd(msgQueue,&message,sizeof(message.contents),0);
		//char* byeMsg = (char*)"goodbye! ";
		//strcat(byeMsg,"child id: ");
		//char* cID;cID[0] = *(buffers + index);
		//strcat(byeMsg,cID);strcat(byeMsg," PID: ");
		//cID[0]=theFork[index];strcat(byeMsg,cID);
	
		// cout<<"sending:";cout<<" child ID: ";cout<<message.contents.childId;cout<<" message. \n";
                //msg->mtype = 1;
		//strcpy(msg->mtext,byeMsg);
		//cout<<byeMsg;
		//msgsnd(msgQueue,msg,1024,IPC_NOWAIT);					


		exit(1);//exits child successfully
	}
	else{

	  }
	//cout<<*(buffers);cout<<"\n";

}
//for(int i=0;i<buffersNo;i++){
//waitpid(theFork[i],0,-1);
//}
//wait on all children

//new msgbuffer to recieve
struct buffer recieve;
int finished = 0;int errorNo=0;int wErrorNo=0;
while(finished<buffersNo/2){//still using buffersNo/2 since that should be the number of forked children. Essentially this loops through and as long as things didn't finish, keep going.
	//cout<<"loop";cout<<"\n";	
	msgrcv(msgQueue,&recieve,sizeof(recieve.contents),0,0);
	
	if(recieve.mtype==3){//error msg
		cout<<"read error! child ID: ";cout<<recieve.contents.childId;cout<<"\n";
		errorNo++;
	}

	if(recieve.mtype==2){//3 is exit msg
		cout<<"goodbye! ";cout<<"child ID: ";cout<<recieve.contents.childId;cout<<" PID: ";cout<<recieve.contents.PID;cout<<"\n";
		finished++;//exited
	}//cout<<"child ";cout<<i;cout<<" stopped \n";
	for(int z=0;z<buffersNo;z++){
	waitpid(theFork[z],0,-1);
	}
}



//check for writing errors (race conditions)
//int raceCheck = (1 << buffersNo/2)-1;//this is all 1s in the right most child for write error checking
//for (int a=0;a<buffersNo/2;a++){
//if((raceCheck-*(buffers+a))!=0){
//	wErrorNo++;
//	cout<<"write error! child ID: ";cout<<*(buffers+a);cout<<"\n";
//}
//}
//cout<<"all children finished";
//for(int a=0;a<buffersNo/2;a++){
//	if(buffers[a]!=0){
//		cout<<"child ID: ";cout<<buffers[a];cout<<"\n";
//}
//}
cout<<"number of read errors ";cout<<errorNo;cout<<" number of write errors ";cout<<wErrorNo;cout<<"\n";
//clean up some memory
shmdt(buffers);
shmctl(memId,IPC_RMID,NULL);

//clean up message queues
msgctl(msgQueue,IPC_RMID,NULL);
}
