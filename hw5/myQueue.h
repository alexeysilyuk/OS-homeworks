//
// Created by alex on 5/2/18.
//

#ifndef HW5_MYQUEUE_H
#define HW5_MYQUEUE_H
#include<iostream>
#include<cstdlib>
#include <string>
#include "RWLock.h"

using namespace std;
int pops=0,pushs=0;

class myQueue
{
    struct request {
        string hostname;
    };

private:
        int size;
        request* requests;
        int head;
        int tail;
        RWLock rwLock;
        pthread_cond_t requester,resolver;
        pthread_mutex_t producer,consumer;

public:
        myQueue(int x=10):size(x),requests(new request[size]),head(-1),tail(-1){
            pthread_cond_init (&requester, NULL);
            pthread_cond_init (&resolver, NULL);
            pthread_mutex_init(&producer,NULL);
            pthread_mutex_init(&consumer,NULL);
        }

        bool push(string _hostname)
        {
            bool b = 0;
//            rwLock.WriteLock();
            pthread_mutex_lock(&producer);
            while(isFull()){
                pthread_cond_wait(&requester,&producer);
            }

            if(isEmpty()){
                tail = head = 0;
                b = true;
            }
            else{
                tail = (tail + 1) % size;
                b = true;
            }
            request file;
            file.hostname=_hostname;
            requests[tail] = file;
            pthread_cond_signal(&resolver);
            pthread_mutex_unlock(&producer);
//            rwLock.WriteUnlock();
            return b;
        }

        string pop()
        {
//            rwLock.ReadLock();
            pthread_mutex_lock(&consumer);
            string val;
            while(isEmpty())
            {
                pthread_cond_wait(&resolver,&consumer);
            }

            if(head == tail){
                val = requests[head].hostname;
                head = -1;
                tail  = -1;
            }
            else
            {
                val = requests[head].hostname;
                head = ( head + 1 ) % size;
            }
            pthread_cond_signal(&requester);
            pthread_mutex_unlock(&consumer);
//            rwLock.ReadUnlock();

            return val;
        }
    bool isEmpty(){  return (tail == -1 && head == -1   ) ? true : false;  }
    bool isFull(){   return ((tail + 1) %  size == head ) ? true : false;   }


    ~myQueue(){
        delete [] requests;

    }

};



#endif //HW5_MYQUEUE_H
