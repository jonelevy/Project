/* synchTester.cpp

	Written Dec 2011 by John Bell for CS 385 
*/

#include <iostream>
#include <cstdlib>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <time.h>

using namespace std;


// Define the data type for a message buffer

struct msgContent {
	int childID, initial, final, whichBuffer;
	char text[ 80 ];
};
struct msgbuffer {
	long mtype; // 1 - Created, 2 - error occured, 3 - string sent, 99 - exited
	//char mtext[ sizeof( struct msgContent ) ];
	struct msgContent content;
};

const int CREATED = 1, ERROR = 2, TEXT = 3, EXITED = 99;

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



int main( int argc, char ** argv ) {

	bool lock = false, timing = false;
	int nBuffers = 13, nChildren, *buffers(NULL), shmID, msqID, pid, semID, sleepRate = 10000;
	union semun semArgs;
	double startTime = time( NULL );

	if( argc >= 2 )
		nBuffers = atoi( argv[ 1 ] );

	if( argc >= 3 ) {
		if( !strcmp( argv[ 2 ], "-lock" ) )
			lock = true;
		else if( !strcmp( argv[ 2 ], "-time" ) )
			timing = true;
	}

	if( argc >= 4 ) {
		if( !strcmp( argv[ 3 ], "-lock" ) )
			lock = true;
		else if( !strcmp( argv[ 3 ], "-time" ) )
			timing = true;
	}

	nChildren = nBuffers / 2;
	
	cout << "\nRunning simulation for " << nChildren << " children accessing " 
         << nBuffers << " buffers \nwith";
    if( !lock ) 
        cout << "out";
    cout << " locking and with";
    if( !timing ) 
        cout << "out";
    cout << " timing considerations.\n\n";
    
	int pids[ nChildren ];
	unsigned short semArray[ nBuffers ];


	// First to set up the shared memory

	shmID = shmget( IPC_PRIVATE, nBuffers * sizeof( int ), 0600 | IPC_CREAT ); 
	buffers = ( int * ) shmat( shmID, NULL, 0 );
	for( int i = 0; i < nBuffers; i++ )
		buffers[ i ] = 0;
 
	// Next to set up the message queue

	msqID = msgget( IPC_PRIVATE, 0600 | IPC_CREAT );

	// If we're using semaphores, set them up too.

	if( lock ) {
		//cerr << "Initializing semaphores.\n";
		semID = semget( IPC_PRIVATE, nBuffers, 0600 | IPC_CREAT );
		semArgs.array = semArray;
		for( int i = 0; i < nBuffers; i++ )
			semArgs.array[ i ] = 1;
		semctl( semID, 0, SETALL, semArgs );
		//semctl( semID, 0, GETALL, semArgs );
		//cerr << "Semaphores initialized to ";
		//for( int i = 0; i < nBuffers; i++ )
		//	cerr << semArgs.array[ i ] << " ";
		//cerr << endl;
	}

	// Fork kids, and put them to work

	for( int i = 0; i < nChildren; i++ ) {

		pid = fork( );

		if( pid < 0 ) {
			cerr << "Error forking\n";
			exit( -1 );
		} else if ( pid > 0 ) { // Parent
			pids[ i ] = pid;
		} else {  // Child

			int ID = i + 1;
			struct msgbuffer sendingMessage;
			struct sembuf semBuffer[ 1 ];
			//struct msgContent *content = &sendingMessage.mtext;

			// Notify parent we have been created

			sendingMessage.mtype = CREATED;
			sendingMessage.content.childID = ID;
			msgsnd( msqID, &sendingMessage, sizeof( struct msgContent ), 0 ); 


			// Now to do the work

			int whichBuf = 0, initialValue, finalValue;

			for( int j = 0; j < nBuffers; j++ ) {

				// First Read

				whichBuf = ( whichBuf + ID ) % nBuffers;
				if( lock ) {
					semBuffer[ 0 ].sem_num = whichBuf;
					semBuffer[ 0 ].sem_op = -1;
					semBuffer[ 0 ].sem_flg = 0;
					//cerr << "Child " << ID << " waiting on " << whichBuf << endl;
					semop( semID, semBuffer, 1 ); 
					//cerr << "Child " << ID << " done waiting on " << whichBuf << ": ";
					//semctl( semID, 0, GETALL, semArgs );
					//for( int i = 0; i < nBuffers; i++ )
					//	cerr << semArgs.array[ i ] << " ";
					//cerr << endl;
				}
				//cerr << "Child " << ID << " getting ready to sleep\n";
				initialValue = buffers[ whichBuf ];
				//cerr << "Child " << ID << " sleeping\n";
				usleep( sleepRate * ID );
				//cerr << "Child " << ID << " woke up.\n";
				finalValue = buffers[ whichBuf ];
				if( initialValue != finalValue ) {
					sendingMessage.mtype = ERROR;
					sendingMessage.content.initial = initialValue;
					sendingMessage.content.final = finalValue;
					sendingMessage.content.whichBuffer = whichBuf;
					msgsnd( msqID, &sendingMessage, sizeof( struct msgContent ), 0 ); 
				}
				
				if( lock ) {
					//cerr << "Child " << ID << " unlocking " << whichBuf << endl;
					semBuffer[ 0 ].sem_num = whichBuf;
					semBuffer[ 0 ].sem_op = 1;
					semBuffer[ 0 ].sem_flg = 0;
					semop( semID, semBuffer, 1 ); 
					//cerr << "Child " << ID << " done unlocking " << whichBuf << endl;
				}


				// Second Read

				whichBuf = ( whichBuf + ID ) % nBuffers;
				if( lock ) {
					semBuffer[ 0 ].sem_num = whichBuf;
					semBuffer[ 0 ].sem_op = -1;
					semBuffer[ 0 ].sem_flg = 0;
					//cerr << "Child " << ID << " waiting on " << whichBuf << endl;
					semop( semID, semBuffer, 1 ); 
					//cerr << "Child " << ID << " done waiting on " << whichBuf << endl;
				}
				initialValue = buffers[ whichBuf ];
				usleep( sleepRate * ID );
				finalValue = buffers[ whichBuf ];
				if( initialValue != finalValue ) {
					sendingMessage.mtype = ERROR;
					sendingMessage.content.initial = initialValue;
					sendingMessage.content.final = finalValue;
					sendingMessage.content.whichBuffer = whichBuf;
					msgsnd( msqID, &sendingMessage, sizeof( struct msgContent ), 0 ); 
				}
				if( lock ) {
					semBuffer[ 0 ].sem_num = whichBuf;
					semBuffer[ 0 ].sem_op = 1;
					semBuffer[ 0 ].sem_flg = 0;
					semop( semID, semBuffer, 1 ); 
				}


				//  Write

				whichBuf = ( whichBuf + ID ) % nBuffers;
				if( lock ) {
					semBuffer[ 0 ].sem_num = whichBuf;
					semBuffer[ 0 ].sem_op = -1;
					semBuffer[ 0 ].sem_flg = 0;
					//cerr << "Child " << ID << " waiting on " << whichBuf << endl;
					semop( semID, semBuffer, 1 ); 
					//cerr << "Child " << ID << " done waiting on " << whichBuf << endl;
				}
				initialValue = buffers[ whichBuf ];
				usleep( sleepRate * ID );
				finalValue = initialValue | ( 1 << ( ID - 1 ) );
				buffers[ whichBuf ] = finalValue;
				if( lock ) {
					semBuffer[ 0 ].sem_num = whichBuf;
					semBuffer[ 0 ].sem_op = 1;
					semBuffer[ 0 ].sem_flg = 0;
					semop( semID, semBuffer, 1 ); 
				}

			} // for each child writing to each buffer


			// Notify parent we are exiting

			sendingMessage.mtype = EXITED;
			msgsnd( msqID, &sendingMessage, sizeof( struct msgContent ), 0 ); 

			exit( 0 ); 
		} // if-else on result of fork( )

	} // for loop through children.  Only the parent exits this loop.
	
	struct msgbuffer messageReceived;
	//struct msgContent *receivedContent = &messageReceived.mtext;
	int nFinished = 0, nReadErrors = 0, nWriteErrors = 0, diff, negDiff;
	struct rusage usageData;
	double userTime = 0.0, systemTime = 0.0;

	while( nFinished < nChildren ) {

		msgrcv( msqID, &messageReceived, sizeof( struct msgContent ), 0, 0 ); 

		switch( messageReceived.mtype ) {

		case CREATED:
			//cout << "Child number " << messageReceived.content.childID << " created\n";
			break;

		case ERROR:
			nReadErrors++;
			if( timing )
				break;
			int initial, final;
			initial = messageReceived.content.initial;
			final = messageReceived.content.final;
			cout << "Child number " << messageReceived.content.childID << " reported change from "
				<< initial << " to " << final << " in buffer " << messageReceived.content.whichBuffer;
			diff = final & ~initial;	// Bits written
			negDiff = initial & ~final;	// Bits overwritten ( erased )
			//cout << " " << diff << " " << negDiff;
			cout << ".  Bad bits = ";
			for( int j = 0; j < nChildren; j++ ) {

				if( diff & ( 1 << j ) )
					cout << j << " ";
				else if( negDiff & ( 1 << j ) )
					cout << -j << " ";

			}

			cout << endl;
			break;

		case TEXT:
			cout << "Child number " << messageReceived.content.childID << " reported:\n";
			cout << messageReceived.content.text << endl;
			break;

		case EXITED:
			//cout << "Child number " << messageReceived.content.childID << " exited\n";
			nFinished++;
			pid = wait4( pids[ messageReceived.content.childID - 1 ], NULL, 0, &usageData );
			//cerr << pid << " finished.\n";
			userTime += usageData.ru_utime.tv_sec + usageData.ru_utime.tv_usec / 1000000.0;
			systemTime += usageData.ru_stime.tv_sec + usageData.ru_stime.tv_usec / 1000000.0;
			break;

		} // switch on message type

	} // While not all children finished.

	// check results
	
	cout << endl;

	int answer = ( 1 << nChildren ) - 1;

	for( int i = 0; i < nBuffers; i++ ) {

		diff = answer - buffers[ i ];
		if( diff ) {
			nWriteErrors++;

			if( !timing ) {
				cout << "Error in buffer " << i << ".  Bad bits = ";

				for( int j = 0; j < nChildren; j++ ) {

					if( diff & ( 1 << j ) )
						cout << j << " ";

				}

				cout << endl;
			} // if not timing

		} // if diff != 0

	} // Loop checking buffers

	cout << endl << nReadErrors << " Read errors and " << nWriteErrors << " write errors encountered in ";

	cout << userTime << "(u)+" << systemTime << "(s) / " << time(NULL) - startTime << " seconds.\n\n";

	// Clean up shared memory
	
	shmdt(buffers );
	shmctl( shmID, IPC_RMID, NULL );

	// Clean up the message queue

	 msgctl( msqID, IPC_RMID, NULL );

} // main
