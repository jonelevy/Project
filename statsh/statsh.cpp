#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

using namespace std;
int main(){
//char *strtok( char *str1, const char *str2 ); 
char *currentCommand=NULL;//initialized as null
double userStats[2048];double systStats[2048];string listOfCommands[2048];//should be fine as long as less than 2048 processes were run before exiting.
string theCommand;
bool exiting=0;
bool stats=0;//flag if stats is the command

struct rusage rusage;
struct timeval userEnd,systemEnd,userStart,systemStart;//will tell us user and system start and end times
int* status;
int statsIndex=0;//this keeps track of what command you are on. Its only used for adding stats to the array.
static struct timeval reset;//used for resetting all of the timevals
int c=0;//yet another index
cout<<"statsh by Jonathan Levy jlevy5\n";
	cout<<"Issue a command";
	cout<<"\n<";

do{
stats=0;
int pipeIO[2];
int in=-1;int out=-1;int next=-1;
userStart=reset;
userEnd=reset;
systemStart=reset;
systemEnd=reset;
//--------------------------input-------------------------------------------------	
	getline(cin,theCommand);
	if(theCommand.size()>0){
	char *commands=new char[theCommand.size()+1];//will store all given commands
	char *args[theCommand.size()+1];//this is supposed to hold all of the parsed words
	char* pipedCommandList[2048];//very large max for commands
	char* pipedCommands[theCommand.size()+1];//the difference between this and the list is pipedCommandList is the command and args while this one is a list of 		the commands
	bool backgroundFlagged [theCommand.size()+1];//if a command needs to be flagged as a background process, a 1 is put into the same index as the command.
	for(int z=0;z<theCommand.size()+1;z++){
		backgroundFlagged[z]=0;//just to make sure we have no confusions here
	}
	int numOfCommands=0;//this is how many commands are in due to piping
	bool hasBackground=0;	
	//cout<<theCommand;cout<<"\n";
	strcpy(commands,theCommand.c_str());		
	

//---------------------------pipe setup-------------------------------------------
	currentCommand=strtok(commands,"|");
	while(currentCommand!=NULL){
		pipedCommandList[numOfCommands]=strdup(currentCommand);//this sets up an array of cmd args as well as keeps an int handy to know how many piped commands we have
		numOfCommands++;
		currentCommand=strtok(NULL,"|");
	}
	for(int b=0;b<=numOfCommands;b++){
		pipedCommands[numOfCommands]=strtok(pipedCommandList[numOfCommands]," ");//this takes the piped commands, and puts in another array just the commands
		//cout<<pipedCommands[numOfCommands];cout<<"\n";
	}
	pid_t theForks[numOfCommands];
	
	
		
//-------------------------------i/o redirection and background processing-------------------------------------------------------------------------------------------

	for(int a=0;a<numOfCommands;a++){
		if(pipedCommandList[a][strlen(pipedCommandList[a])-1]=='&'){
			backgroundFlagged[a]=1;
			pipedCommandList[a][strlen(pipedCommandList[a])-1]='\0';
		}
}

bool inFlag[numOfCommands];bool outFlag[numOfCommands];char* inFile[numOfCommands];char*outFile[numOfCommands];
//--------------------------strtok and input handling------------------------------
	currentCommand=strtok(commands," ");
	//char* newCommand = currentCommand;//sets the actual command to its own char*
		if(strcmp(currentCommand,"exit")==0){//only trigger the flag if the next command was exit.
		exiting=1;
	}
	if(strcmp(currentCommand,"stats")==0)
		stats=1;

	for(int i=0;i<numOfCommands;i++){

		currentCommand=strtok(pipedCommandList[i]," ");
			if(strcmp(currentCommand,"exit")==0){//only trigger the flag if the next command was exit.
		exiting=1;
	}
	if(strcmp(currentCommand,"stats")==0)
		stats=1;
	if(exiting==0)//all this does is if the command isn't exit, proceed with everything else.
	{		
	if(stats==0){
		char* newCommand = currentCommand;//sets the actual command to its own char*
		c=0;
		//memset(args,'\0',theCommand.size());
		for(int a=0;a<theCommand.size()+1;a++){
			args[a]=NULL;//reset all of args to empty essentially otherwise errors occur from commands with no args
		}		
		while(currentCommand!=NULL){
			args[c]=strdup(currentCommand);
			c++;		
			//cout<<currentCommand;cout<<" \n";
			currentCommand=strtok(NULL," ");
		}
		for(int z=0;z<c;z++){
		if(args[z][0]=='>'){
		outFile[numOfCommands]=args[z+1];
		args[z]=NULL;
		args[z+1]=NULL;
		freopen(outFile[numOfCommands],"w",stdout);
		break;
		}
		if(args[z][0]=='<'){
		inFile[numOfCommands]=args[z+1];
		args[z]=NULL;
		//args[z+1]=NULL;
		freopen(inFile[numOfCommands],"r",stdin);
		break;
		}
		}
		
		//for(int a=0;a<theCommand.size()+1;a++){
		//	cout<<args[a];cout<<"\n";
		//}
		

//-----------------Fork and execute-----------------------------------------------

//for(int g=0;g<numOfCommands;g++){
	//cout<<pipedCommandList[g];cout<<"\n";
//}	

		//theForks[i]=fork();//creates the new process for exec to load into
		//cout<<"forking process";cout<<i;cout<<" \n";
		//getrusage(RUSAGE_CHILDREN,&rusage);
		//userStart=rusage.ru_utime;
		//systemStart=rusage.ru_stime;
		theForks[i]=fork();
		
		//cout<<"fork value is ";cout<<theForks[i];cout<<"\n";		
		if(theForks[i]<0){//fork failed
			cout<<"something happened and the fork failed. \n";
			exit(1);
		}
		
			if(theForks[i]==0){//success for the fork
			

				if(i!=0){
					close(in);
					close(out);
					in=next;
				}
				if(i!=numOfCommands-1){//if i does not equal the number of commands, we got commands to go
				pipe(pipeIO);
				out=pipeIO[1];
				next=pipeIO[0];
				}
				else if(i=numOfCommands-1)
					next=out=-1;
				if(next>=0)
					close(next);
				if(in>=0){
					dup2(in,0);
					//dup2(out,1);
					close(in);
				}
				if(out >= 0) {
					dup2(out, 1);
					close(out);
				}
				
				//cout<<"forking/execing command";
				if(execvp(newCommand,args)<0){//this checks if execute successfully.
					cout<<"something went wrong with the last command. \n";
					exit(0);
				}	
		}

	}
	for(int i=0;i<numOfCommands;i++){		
		if(backgroundFlagged[i]==1){
			wait4(theForks[i],&status,WNOHANG,&rusage);//if it was flagged as background process
		}
		else{
			wait4(theForks[i],&status,0,&rusage);//executes the process of wait4
		}
			userEnd=rusage.ru_utime;
			systemEnd=rusage.ru_stime;
		
		//cout<<"process ";
		//cout<<pipedCommandList[i];
		userStats[statsIndex]=userEnd.tv_sec+.000001*userEnd.tv_usec;
		systStats[statsIndex]=systemEnd.tv_sec+.000001*systemEnd.tv_usec;
		listOfCommands[statsIndex]=pipedCommandList[i];
		cout<<"command: ";
		cout<<listOfCommands[statsIndex];
		cout<<", user runtime: ";
		cout<<userStats[statsIndex];
		cout<<" s, system runtime: ";
		cout<<systStats[statsIndex];
		cout<<" s \n";
		statsIndex++;
	}
//--------------------------stats printing-------------------------------------------
	
	//}		//getrusage(RUSAGE_SELF,&rusage);
	}
	if(stats==1){
		for(int z=0;z<statsIndex;z++){//since I haven't used z as an index
			cout<<"command: ";
			cout<<listOfCommands[z];
			cout<<", user runtime: ";
			cout<<userStats[z];
			cout<<" s, system runtime: ";
			cout<<systStats[z];
			cout<<" s \n";	
		}
	}
			//cout<<"process ";
			//cout<<newCommand;
			//cout<<"\n user up time: ";cout<<userEnd.tv_sec+(userEnd.tv_usec/1000000);cout<<" seconds \n";
			//cout<<"system up time: ";cout<<systemEnd.tv_sec+(systemEnd.tv_usec/1000000);cout<<" seconds \n";
			cout<<"Issue a command\n";
			cout<<"<";
			
	}
	//cout<<"end loop";
}
//}
}while(exiting==0);//as long as the command is not "exit", after a command is issued it will ask you for another one.
	for(int z=0;z<statsIndex;z++){//since I haven't used z as an index
			cout<<"command: ";
			cout<<listOfCommands[z];
			cout<<", user runtime: ";
			cout<<userStats[z];
			cout<<" s, system runtime: ";
			cout<<systStats[z];
			cout<<" s \n";	
		
		}	

	cout<<"Exiting. Have a nice day. \n";
}
