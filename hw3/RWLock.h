//
// Created by alex on 4/20/18.
// This class implemented according to WIKIPEDIA pseudo algo
//

#ifndef HW3_RWLOCK_H
#define HW3_RWLOCK_H

#include <iostream>
#include "Semaphore.h"
using namespace std;
class RWLock {
public:
    // get 4 semaphores,
    RWLock(int rmutex,int wmutex,int readTry,int resource)
    {
        this->rmutex=rmutex;
        this->wmutex=wmutex;
        this->readTry=readTry;
        this->resource=resource;
        readcount=-1;
        writecount=-1;
    }

    void ReadLock() {
        p(readTry);
        p(rmutex);
        readcount++;
        if(readcount==1)
            p(resource);

        v(rmutex);
        v(readTry);
    }

    void ReadUnlock() {
        p(rmutex);
        readcount--;
        if(readcount==0)
            v(resource);
        v(rmutex);
    }

    void WriteLock() {
        p(wmutex);
        writecount++;
        if(writecount==1)
            p(readTry);
        v(wmutex);
    }

    void WriteUnlock() {
        p(wmutex);
        writecount--;
        if(writecount==0)
            v(readTry);
        v(wmutex);
    }


private:
    int rmutex, wmutex, readTry, resource;
    int readcount, writecount;

};
#endif //HW3_RWLOCK_H
