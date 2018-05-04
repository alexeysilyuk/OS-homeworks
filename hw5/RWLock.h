//
// Created by alex on 4/20/18.
// This class implemented according to WIKIPEDIA pseudo algo
//

#ifndef HW3_RWLOCK_H
#define HW3_RWLOCK_H

#include <iostream>
#include <pthread.h>
using namespace std;
class RWLock {
public:
    RWLock()
    {
        pthread_mutex_init(&rmutex,NULL);
        pthread_mutex_init(&wmutex,NULL);
        pthread_mutex_init(&readTry,NULL);
        pthread_mutex_init(&resource,NULL);
        readcount=-1;
        writecount=-1;
    }
    ~RWLock(){
        pthread_mutex_destroy(&rmutex);
        pthread_mutex_destroy(&wmutex);
        pthread_mutex_destroy(&readTry);
        pthread_mutex_destroy(&resource);
    }

    void ReadLock() {
        pthread_mutex_lock(&readTry);
        pthread_mutex_lock(&rmutex);
        readcount++;
        if(readcount==1)
            pthread_mutex_lock(&resource);

        pthread_mutex_unlock(&rmutex);
        pthread_mutex_unlock(&readTry);
    }

    void ReadUnlock() {
        pthread_mutex_lock(&rmutex);
        readcount--;
        if(readcount==0)
            pthread_mutex_unlock(&resource);
        pthread_mutex_unlock(&rmutex);
    }

    void WriteLock() {
        pthread_mutex_lock(&wmutex);
        writecount++;
        if(writecount==1)
            pthread_mutex_lock(&readTry);
        pthread_mutex_unlock(&wmutex);
    }

    void WriteUnlock() {
        pthread_mutex_lock(&wmutex);
        writecount--;
        if(writecount==0)
            pthread_mutex_unlock(&readTry);
        pthread_mutex_unlock(&wmutex);
    }


private:
    pthread_mutex_t rmutex, wmutex, readTry, resource;
    int readcount, writecount;

};
#endif //HW3_RWLOCK_H
