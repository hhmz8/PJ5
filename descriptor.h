#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H
#include "structs.h"

void initDescriptor(struct shmseg* shmp, int resourceSize, int processSize);

void allocateResource(struct shmseg* shmp, int request[], int resourceSize, int processSize, int processIndex);
void freeResource(struct shmseg* shmp, int resourceSize, int processSize, int processIndex);
void setAllocationVector(struct shmseg* shmp, int resourceSize, int processSize);
int isRequestValid(struct shmseg* shmp, int request[], int size);

void printDescriptor(struct shmseg* shmp, int size);

#endif