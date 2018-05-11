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
#define DUMPER_THREADS 3
#define QUEUE_SIZE 5

myQueue* globalQueue = new myQueue(QUEUE_SIZE);
ResultsArray* resultsArray = new ResultsArray();

bool allRequestersDone=false;
unsigned long **requestsDone;
int totalFiles;






#endif //HW5_GLOBALQUEUE_H
