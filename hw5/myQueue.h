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

public:
        myQueue(int x=10):size(x),requests(new request[size]),head(-1),tail(-1){

        }

        bool push(string _hostname)
        {
            bool b = 0;
            rwLock.WriteLock();
            if(isFull())
            {
                rwLock.WriteUnlock();
                return false;
            }
            else if(isEmpty()){
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
            rwLock.WriteUnlock();
            return b;
        }

        string pop()
        {
            rwLock.ReadLock();
            string val;
            if(isEmpty())
            {
                rwLock.ReadUnlock();
                return val;
            }
            else if(head == tail){
                val = requests[head].hostname;
                head = -1;
                tail  = -1;
            }
            else
            {
                val = requests[head].hostname;
                head = ( head + 1 ) % size;
            }
            rwLock.ReadUnlock();

            return val;
        }
    bool isEmpty(){  return (tail == -1 && head == -1   ) ? true : false;  }
    bool isFull(){   return ((tail + 1) %  size == head ) ? true : false;   }


    ~myQueue(){
        delete [] requests;

    }

};



#endif //HW5_MYQUEUE_H
