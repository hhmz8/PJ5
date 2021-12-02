/*
Hanzhe Huang
11/1/2021
userprocess.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <time.h>
#include <ctype.h> //isprint
#include <unistd.h> //sleep
#include "structs.h"
#include "clock.c"

#define MAX_NS 50000000 // Max wait time between requests

// Reference: https://www.tutorialspoint.com/c_standard_library/c_function_rand.htm
int main(int argc, char** argv){
	pid_t pid = getpid();
	int i;
	int pidIndex = 0;
	time_t t;
	srand((unsigned) time(&t) ^ (getpid()<<16)); // Reference: https://stackoverflow.com/questions/8623131/why-is-rand-not-so-random-after-fork
	
	struct clock requestTime = {0, 0};
	const int newRequestSecs = 0;
	const int newRequestNS = 10000000;
	int hasResources = 0;
	
	int msgid = msgget(MSG_KEY, 0666 | IPC_CREAT);
	
	struct shmseg *shmp;
    int shmid = shmget(SHM_KEY, BUF_SIZE, 0666|IPC_CREAT);
	if (shmid == -1) {
		perror("Error: shmget");
		exit(-1);
	}
	shmp = shmat(shmid, 0, 0);
	msg_t.pid = pid;
	
	struct clock terminationTime;
	terminationTime.clockSecs = shmp->ossclock.clockSecs+1;
	terminationTime.clockNS = 0;
	
	for (i = 0; i < MAX_PRO; i++){
		if (shmp->processTable[i].processPid == pid){
			pidIndex = i;
		}
	}
	
	while (1 == 1){
		if (isClockLarger(shmp->ossclock, requestTime) == 0){
			if (newRequestSecs != 0){
				requestTime.clockSecs = shmp->ossclock.clockSecs + (rand() % newRequestSecs);
			}
			else {
				requestTime.clockSecs = shmp->ossclock.clockSecs + 0;
			}
			requestTime.clockNS = shmp->ossclock.clockNS + (rand() % newRequestNS);
			if (requestTime.clockNS >= 1000000000){
				requestTime.clockSecs++;
				requestTime.clockNS -= 1000000000;
			}
			
			if ((hasResources == 0 || (rand() % 2 == 0)) && isClockLarger(shmp->ossclock, terminationTime) != 0){
				
				// Set message
				msg_t.mtype = 1;
				msg_t.pid = pid;
				
				// Send message
				msgsnd(msgid, &msg_t, sizeof(msg_t), 0);
				
				//Wait for request approval
				msgrcv(msgid, &msg_t, sizeof(msg_t), pid, 0);
				hasResources = 1;
			}
			else {
				// Set message
				msg_t.mtype = 2;
				msg_t.pid = pid;
				
				// Send message
				msgsnd(msgid, &msg_t, sizeof(msg_t), 0);
				
				//Wait for request approval
				msgrcv(msgid, &msg_t, sizeof(msg_t), pid, 0);
				hasResources = 0;
			}
		}
		if (isClockLarger(shmp->ossclock, terminationTime) == 0 && hasResources == 0){
			printf("P%d exited at time %d:%d.\n", pidIndex, shmp->ossclock.clockSecs, shmp->ossclock.clockNS);
			shmp->processTable[pidIndex].processPid = 0;
			exit(-1);
		}
	}
	return 0;
}