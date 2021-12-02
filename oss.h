/*
Hanzhe Huang
10/20/2021
runsim.h
*/

#ifndef OSS_H
#define OSS_H
#define TOTAL_PRO 40

void logexit();
void sigint_parent(int sig);
void sigint(int sig);
void sigalrm(int sig);

void parent();
void child();
void deallocate();

void incrementClockShm(struct shmseg* shmp, int incS, int incNS);

struct shmseg* shmobj();
void initshmobj(struct shmseg* shmp);

int getPidIndex(struct shmseg* shmp, int pid);

#endif