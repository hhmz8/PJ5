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

#define MAX_NS 50000000 // Max wait time between requests

// Reference: https://www.tutorialspoint.com/c_standard_library/c_function_rand.htm
int main(int argc, char** argv){
	pid_t pid = getpid();
	time_t t;
	srand((unsigned) time(&t) ^ (getpid()<<16)); // Reference: https://stackoverflow.com/questions/8623131/why-is-rand-not-so-random-after-fork
	while (1 == 1){
		int msgid = msgget(MSG_KEY, 0666 | IPC_CREAT);
		
		// Wait for message of type pid
		msgrcv(msgid, &msg_t, sizeof(msg_t), pid, 0);
		
		// Set message
		msg_t.mtype = 1;
		
		// Send message
		msgsnd(msgid, &msg_t, sizeof(msg_t), 0);
	}
	return 0;
}