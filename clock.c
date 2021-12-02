#include <stdio.h>
#include <stdlib.h>
#include "structs.h"

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
