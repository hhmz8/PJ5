/*
Hanzhe Huang
11/30/2021
oss.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <time.h> //time
#include <sys/wait.h> //wait
#include <signal.h>
#include <ctype.h> //isprint
#include <unistd.h> //sleep, alarm
#include "queue.c"
#include "structs.h"
#include "descriptor.c"
#include "oss.h"

// Reference: https://www.tutorialspoint.com/inter_process_communication/inter_process_communication_shared_memory.htm
// Reference: https://stackoverflow.com/questions/19461744/how-to-make-parent-wait-for-all-child-processes-to-finish
// Reference: https://www.tutorialspoint.com/inter_process_communication/inter_process_communication_message_waitQueue.htm
// Reference: https://www.geeksforgeeks.org/ipc-using-message-waitQueue/

extern int errno;
static char* logName;

int main(int argc, char** argv) {
	// Signal handlers;
	signal(SIGINT, sigint_parent);
	signal(SIGALRM, sigalrm);
	
	// Interval constants
	const int maxTimeBetweenNewProcsSecs = 1;
	const int maxTimeBetweenNewProcsNS = 50000000;
	
	// Statistics
	int processNum = 0, idle = 1;
	
	// Message queue init
	int msgid = msgget(MSG_KEY, 0666 | IPC_CREAT);
	if (msgid == -1) {
		perror("Error: msgget");
		exit(-1);
	}
	
	FILE* fptr;
	int i;
	int pid = 1, status = 0;
	struct clock tempClock = {0, 0};
	struct clock lastNewProcessTime = {0, 0};
	time_t t;
	srand((unsigned) time(&t));
	
	// Initialize wait queue
	int waitQueue[MAX_PRO];
	initQueue(waitQueue, MAX_PRO);
	
	logName = malloc(200);
	logName = "logfile";
	
	// Clear log file
	fptr = fopen(logName, "w");
	fclose(fptr);
	fptr = fopen(logName, "a");
	
	// Shm Init 
	initshmobj(shmobj());
	initDescriptor(shmobj(),RES_SIZE,MAX_PRO);
	printDescriptor(shmobj(),RES_SIZE);
	
	struct shmseg *shmp;
    int shmid = shmget(SHM_KEY, BUF_SIZE, 0666|IPC_CREAT);
	if (shmid == -1) {
		perror("Error: shmget");
		exit(-1);
	}
	shmp = shmat(shmid, 0, 0);
	
	// Main loop
	while(1){
		idle = 1;
		
		// If process limit isn't reached and clock has advanced, fork a process
		for (i = 0; i < MAX_PRO; i++){
			if (shmp->processTable[i].processPid == 0) {
				break;
			}
		}
		if (i != MAX_PRO && (isClockLarger(shmp->ossclock, lastNewProcessTime) == 0) && processNum < TOTAL_PRO){
			idle = 0;
			pid = fork();
			switch ( pid )
			{
			case -1:
				perror("Error: fork");
				return -1;

			case 0: // Child, terminates
				child();
				break;

			default: // Parent
				// Increment total processes created
				processNum++;
				if (maxTimeBetweenNewProcsSecs != 0){
					lastNewProcessTime.clockSecs = shmp->ossclock.clockSecs + (rand() % maxTimeBetweenNewProcsSecs);
				}
				else {
					lastNewProcessTime.clockSecs = shmp->ossclock.clockSecs + 0;
				}
				lastNewProcessTime.clockNS = shmp->ossclock.clockNS + (rand() % maxTimeBetweenNewProcsNS);
				if (lastNewProcessTime.clockNS >= 1000000000){
					lastNewProcessTime.clockSecs++;
					lastNewProcessTime.clockNS -= 1000000000;
				}
				
				// Store pid to process table, dispatch it
				printf("Child %d forked at %d:%d.\n", pid, shmp->ossclock.clockSecs, shmp->ossclock.clockNS);
				shmp->processTable[i].processPid = pid;
				msg_t.mtype = pid;
				msgsnd(msgid, &msg_t, sizeof(msg_t), 0);
				msgrcv(msgid, &msg_t, sizeof(msg_t), 1, 0);
				idle = 0;
				break;
			}
		}
		
		// Check if no action was taken, increment system clock by a small amount
		if (idle == 1 && !(processNum >= TOTAL_PRO)){
			tempClock.clockSecs = 0;
			tempClock.clockNS = 50000000;
			incrementClockShm(shmobj(), tempClock.clockSecs,  tempClock.clockNS);
		}
		
		// Check if oss is due for termination
		if (processNum >= TOTAL_PRO){
			if (waitpid(0, &status, WNOHANG) < 1){
				fclose(fptr);
				logexit();
				return 0;
			}
		}
	}
	return -1;
}

// Logs termination time
void logexit(){
	FILE* fptr;
	char timeBuffer[40];
	time_t tempTime = time(NULL);
	fptr = fopen(logName, "a");
	strftime(timeBuffer, 40, "%H:%M:%S", localtime(&tempTime));
	printf("Saving: %s %d terminated\n", timeBuffer, getpid());
	fprintf(fptr, "OSS: %s %d terminated\n", timeBuffer, getpid());
	fclose(fptr);
}

// Signals
void sigint_parent(int sig){
	printf("Process %d exiting...\n",getpid());
	deallocate();
	printf("Terminating child processes...\n");
	kill(0, SIGINT);
	parent();
	logexit();
	exit(0);
}

void sigint(int sig){
	kill(0, SIGINT);
	exit(0);
}

void sigalrm(int sig){
	printf("Program timed out.\n");
	kill(0, SIGINT);
	exit(0);
}

// Parent function to wait for children processes
void parent(){
	int childpid;
	while ((childpid = (wait(NULL))) > 0);
	printf("Stopped waiting for children.\n");
}

// Reference: http://www.cs.umsl.edu/~sanjiv/classes/cs4760/src/shm.c
// Reference: https://www.geeksforgeeks.org/signals-c-set-2/
void child(){
	signal(SIGINT, sigint);
	signal(SIGALRM, SIG_IGN);
	
	// Exec user process
	if ((execl("userprocess", "userprocess", (char*)NULL)) == -1){
		perror("Error: execl/stdin");
		exit(-1);
	}
	exit(1);
}

// Deallocates shared memory & message queue
void deallocate(){
	//shm
    int shmid = shmget(SHM_KEY, BUF_SIZE, 0666|IPC_CREAT);
	if (shmid == -1) {
		perror("Error: shmget");
		exit(-1);
	}
	if (shmctl(shmid, IPC_RMID, 0) == -1) {
		perror("Error: shmctl");
		exit(-1);
	}
	//msg
	int msgid = msgget(MSG_KEY, 0666 | IPC_CREAT);
	if (msgid == -1) {
		perror("Error: msgget");
		exit(-1);
	}
	if (msgctl(msgid, IPC_RMID, NULL) == -1) {
		perror("Error: msgctl");
		exit(-1);
	}
	printf("Shared memory & message queue deallocated.\n");
}

// Returns the shared memory segment
struct shmseg* shmobj(){
	struct shmseg *shmp;
    int shmid = shmget(SHM_KEY, BUF_SIZE, 0666|IPC_CREAT);
	if (shmid == -1) {
		perror("Error: shmget");
		exit(-1);
	}
	shmp = shmat(shmid, 0, 0);
	return shmp;
}

// Initializes shared memory segment
void initshmobj(struct shmseg* shmp){
	int i;
	shmp->ossclock.clockSecs = 0;
	shmp->ossclock.clockNS = 0;
	for (i = 0; i < MAX_PRO; i++){
		shmp->processTable[i].processPid = 0;
	}
}

// Increments the clock by seconds and nanoseconds
void incrementClockShm(struct shmseg* shmp, int incS, int incNS){
	shmp->ossclock.clockSecs += incS;
	shmp->ossclock.clockNS += incNS;
	if (shmp->ossclock.clockNS >= 1000000000){
		shmp->ossclock.clockSecs++;
		shmp->ossclock.clockNS -= 1000000000;
	}
}

// Check if clockA is larger than clockB
int isClockLarger(struct clock clockA, struct clock clockB){
	if (clockA.clockSecs > clockB.clockSecs){
		return 0;
	}
	else if (clockA.clockSecs < clockB.clockSecs){
		return -1;
	}
	else if (clockA.clockNS > clockB.clockNS){
		return 0;
	}
	else {
		return -1;
	}
}
