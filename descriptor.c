﻿#include <stdio.h>
#include <stdlib.h>
#include <time.h> //time
#include "structs.h"
#include "descriptor.h"

// Descriptor functions for oss only
void initDescriptor(struct shmseg* shmp, int resourceSize, int processSize){
	int i, j;
	time_t t;
	srand((unsigned) time(&t));
	for (i = 0; i < resourceSize; i++){
		shmp->resourceDescriptor.resourceVector[i] = (rand() % 10) + 1;
		shmp->resourceDescriptor.allocationVector[i] = 0;
	}
	for (i = 0; i < processSize; i++){
		for (j = 0; j < resourceSize; j++){
			shmp->resourceDescriptor.requestMatrix[i][j] = 0;
			shmp->resourceDescriptor.allocationMatrix[i][j] = 0;
		}
	}
}

void allocateResource(struct shmseg* shmp, int request[], int resourceSize, int processSize, int processIndex){
	int i;
	for (i = 0; i < resourceSize; i++){
		shmp->resourceDescriptor.allocationMatrix[processIndex][i] = request[i];
	}
	setAllocationVector(shmp, resourceSize, processSize);
}

void setAllocationVector(struct shmseg* shmp, int resourceSize, int processSize){
	int i, j;
	int sum;
	for (i = 0; i < resourceSize; i++){
		sum = 0;
		for (j = 0; j < processSize; j++){
			sum += shmp->resourceDescriptor.allocationMatrix[i][j];
		}
		sum -= shmp->resourceDescriptor.resourceVector[i];
		shmp->resourceDescriptor.allocationVector[i] = sum;
	}
}

int isRequestValid(struct shmseg* shmp, int request[], int size){
	int i;
	for (i = 0; i < size; i++){
		if (shmp->resourceDescriptor.allocationVector[i] - request[i] < 0)
			return -1;
	}
	return 0;
}

// Debug
void printDescriptor(struct shmseg* shmp, int size){
	int i;
	for (i = 0; i < size; i++){
		printf("%d\n",shmp->resourceDescriptor.resourceVector[i]);
	}
}