//
// Created by alex on 5/3/18.
//

#ifndef HW5_GLOBALQUEUE_H
#define HW5_GLOBALQUEUE_H

#include "myQueue.h"
#include "ResultsArray.h"
int globalargc;


#define MAX_IP_ADRESSES 10
#define MAX_RESOLVER_THREADS 10
#define MIN_RESOLVER_THREADS 2

myQueue* globalQueue = new myQueue(10);
ResultsArray* resultsArray = new ResultsArray();

bool allRequestersDone=true;
unsigned long **requestsDone;
int totalFiles;
#endif //HW5_GLOBALQUEUE_H
