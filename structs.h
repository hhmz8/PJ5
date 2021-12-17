#ifndef STRUCTS_H
#define STRUCTS_H
#define BUF_SIZE 1024
#define SHM_KEY 806040
#define MSG_KEY 806041
#define RES_SIZE 20
#define MAX_PRO 18
#define LOGFILE "logfile"

struct clock {
	int clockSecs;
	int clockNS;
};

struct process {
	int processPid;
};

struct descriptor {
	int resourceVector[RES_SIZE];
	int allocationVector[RES_SIZE];
	int requestMatrix[MAX_PRO][RES_SIZE];
	int allocationMatrix[MAX_PRO][RES_SIZE];
	int request[RES_SIZE];
};

// Process control block
struct shmseg {
	char buf[BUF_SIZE];

	// Test Count
	int numberProcesses;
   
	// Clock
	struct clock ossclock;
   
	// Process table
	struct process processTable[MAX_PRO];
	
	// Resources
	struct descriptor resourceDescriptor;
};

struct msgbuf {
	long mtype;
	int pid;
} msg_t;

#endif