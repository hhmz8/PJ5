#include <stdio.h>
#include <stdlib.h>
#include <time.h> //time
#include "structs.h"
#include "descriptor.h"

// Descriptor functions for oss only
void initDescriptor(struct shmseg* shmp, int resourceSize, int processSize){
	int i, j;
	time_t t;
	srand((unsigned) time(&t) ^ (shmp->ossclock.clockNS<<16));
	for (i = 0; i < resourceSize; i++){
		shmp->resourceDescriptor.resourceVector[i] = (rand() % 10) + 5;
		shmp->resourceDescriptor.allocationVector[i] = 0;
	}
	for (i = 0; i < processSize; i++){
		for (j = 0; j < resourceSize; j++){
			shmp->resourceDescriptor.requestMatrix[i][j] = 0;
			shmp->resourceDescriptor.allocationMatrix[i][j] = 0;
		}
	}
	setAllocationVector(shmp, resourceSize, processSize);
}

// Allocate resources for a process
int allocateForProcess(struct shmseg* shmp, int resourceSize, int processSize, int processIndex){
	setRequest(shmp, resourceSize);
	if (isRequestValid(shmp, shmp->resourceDescriptor.request, resourceSize) == 0){
		allocateResource(shmp, shmp->resourceDescriptor.request, resourceSize, processSize, processIndex);
		printf("P%d allocated.\n", processIndex);
		return 0;
	}
	else
		printf("Recieved request from P%d, Resource unavaliable.\n", processIndex);
	return -1;
}

// Allocate resources for a process
void allocateResource(struct shmseg* shmp, int request[], int resourceSize, int processSize, int processIndex){
	int i;
	for (i = 0; i < resourceSize; i++){
		shmp->resourceDescriptor.allocationMatrix[processIndex][i] += request[i];
	}
	setAllocationVector(shmp, resourceSize, processSize);
}

// Free resources for a process 
void freeResource(struct shmseg* shmp, int resourceSize, int processSize, int processIndex){
	int i;
	for (i = 0; i < resourceSize; i++){
		shmp->resourceDescriptor.allocationMatrix[processIndex][i] = 0;
	}
	setAllocationVector(shmp, resourceSize, processSize);
}

// Sets a randomized resource request
void setRequest(struct shmseg* shmp, int resourceSize){
	int i;
	int max;
	time_t t;
	srand((unsigned) time(&t) ^ (shmp->ossclock.clockNS<<16));
	for (i = 0; i < resourceSize; i++){
		//max = shmp->resourceDescriptor.resourceVector[i];
		max = 1;
		shmp->resourceDescriptor.request[i] = (rand() % (max + 1));
	}
}

// Sets the allocation vector based on the matrix
void setAllocationVector(struct shmseg* shmp, int resourceSize, int processSize){
	int i, j;
	int sum;
	for (i = 0; i < resourceSize; i++){
		sum = 0;
		for (j = 0; j < processSize; j++){
			sum += shmp->resourceDescriptor.allocationMatrix[j][i];
		}
		sum = shmp->resourceDescriptor.resourceVector[i] - sum;
		shmp->resourceDescriptor.allocationVector[i] = sum;
	}
}

// Check if resource request is valid
int isRequestValid(struct shmseg* shmp, int request[], int size){
	int i;
	for (i = 0; i < size; i++){
		if (shmp->resourceDescriptor.allocationVector[i] - request[i] < 0)
			return -1;
	}
	return 0;
}

// Debug, prints allocated resources
void printDescriptor(FILE* inptr, struct shmseg* shmp, int resourceSize, int processSize){
	int i, j;
	int shortSize = resourceSize;
	int resource;
	fprintf(inptr,"     ");
	for (i = 0; i < shortSize; i++){
		if (i < 10){
			fprintf(inptr," R%d ",i);
		}
		else {
			fprintf(inptr,"R%d ",i);
		}
	}
	fprintf(inptr,"\n");
	for (i = 0; i < processSize; i++){
		if (i < 10){
			fprintf(inptr," P%d: ",i);
		}
		else {
			fprintf(inptr,"P%d: ",i);
		}
		for (j = 0; j < shortSize; j++){
			resource = shmp->resourceDescriptor.allocationMatrix[i][j];
			if (resource < 10){
				fprintf(inptr,"  %d ",resource);
			}
			else {
				fprintf(inptr," %d ",resource);
			}
		}
		fprintf(inptr,"\n");
	}
	fprintf(inptr,"Resource Vector: \n");
	for (i = 0; i < shortSize; i++){
		resource = shmp->resourceDescriptor.resourceVector[i];
		if (resource < 10){
			fprintf(inptr,"  %d ",resource);
		}
		else {
			fprintf(inptr," %d ",resource);
		}
	}
	fprintf(inptr,"\n");
	fprintf(inptr,"Allocation Vector: \n");
	for (i = 0; i < shortSize; i++){
		resource = shmp->resourceDescriptor.allocationVector[i];
		if (resource < 10){
			fprintf(inptr,"  %d ",resource);
		}
		else {
			fprintf(inptr," %d ",resource);
		}
	}
	fprintf(inptr,"\n");
}