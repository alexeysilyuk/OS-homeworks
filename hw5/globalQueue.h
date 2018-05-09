//
// Created by alex on 5/3/18.
//

#ifndef HW5_GLOBALQUEUE_H
#define HW5_GLOBALQUEUE_H

#include "myQueue.h"
#include "ResultsArray.h"
int globalargc;
struct argvc{
    char* argv[10];
    int argc;
};

myQueue* globalQueue = new myQueue(10);
ResultsArray* resultsArray = new ResultsArray();

int finishedFiles=0,totalFiles;
#endif //HW5_GLOBALQUEUE_H
