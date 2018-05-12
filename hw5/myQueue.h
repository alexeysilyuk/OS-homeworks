//
// Created by alex on 5/2/18.
//

#ifndef HW5_MYQUEUE_H
#define HW5_MYQUEUE_H
#include<iostream>
#include<cstdlib>
#include <string>
#define  DEFAULT_QUEUE_SIZE 10

using namespace std;
struct request {
    string hostname;
    pthread_mutex_t* threadMutex;
    pthread_cond_t* taskDoneCond;
};
pthread_cond_t requestersCondVar;

class myQueue
{


private:
        int size;
        request* requests;
        int head;
        int tail;
        pthread_cond_t requester,resolver;
        pthread_mutex_t producer,consumer;

public:
        // default queue size is 10
        myQueue(int x=DEFAULT_QUEUE_SIZE):size(x),requests(new request[size]),head(-1),tail(-1){
            pthread_cond_init (&requester, NULL);
            pthread_cond_init (&resolver, NULL);
            pthread_mutex_init(&producer,NULL);
            pthread_mutex_init(&consumer,NULL);
        }
    ~myQueue(){
        delete [] requests;
        pthread_cond_destroy(&requester);
        pthread_cond_destroy(&resolver);
        pthread_mutex_destroy(&producer);
        pthread_mutex_destroy(&consumer);
    }
    // pushing to thread safe array
        bool push(string _hostname,pthread_cond_t *cond, pthread_mutex_t *mutex)
        {
            bool b = 0;
            pthread_mutex_lock(&producer);
            if(isFull()){
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
            file.taskDoneCond=cond;
            file.threadMutex=mutex;

            requests[tail] = file;

            pthread_cond_signal(&resolver);
            pthread_mutex_unlock(&producer);

            return b;
        }


        // thread safe pop
        request* pop()
        {
            pthread_mutex_lock(&consumer);
            request *result = new request();


            string val;
            // if queue is empty, go to slip till receive signal
            if(isEmpty())
            {
                pthread_cond_wait(&resolver,&consumer);
            }

            if(head == tail){
                result->hostname = requests[head].hostname;
                result->taskDoneCond=requests[head].taskDoneCond;
                head = -1;
                tail  = -1;
            }
            else
            {
                result->hostname = requests[head].hostname;
                result->taskDoneCond=requests[head].taskDoneCond;
                head = ( head + 1 ) % size;
            }
            // signal requesters to wake up and to add new task
            pthread_cond_signal(&requester);
            pthread_mutex_unlock(&consumer);

            return result;
        }

    // basic functions
    bool isEmpty(){  return tail == -1 && head == -1    ? true : false;  }
    bool isFull(){   return (tail + 1) %  size == head  ? true : false;   }




};



#endif //HW5_MYQUEUE_H
