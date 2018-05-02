//
// Created by alex on 5/2/18.
//

#ifndef HW5_MYQUEUE_H
#define HW5_MYQUEUE_H
#include<iostream>
#include<cstdlib>
#include <string>

using namespace std;


class myQueue
{
    private:
        int size;
        string *request;
        int head;
        int tail;
        pthread_mutex_t mutex;

    public:
        myQueue(int x=10):size(x),request(new string[size]),head(-1),tail(-1){
            pthread_mutex_init(&mutex,NULL);
        }

        bool push(string x)
        {
            bool b = 0;
            if(isFull())
            {
                cout << "Queue is full\n";
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
            pthread_mutex_lock(&mutex);
            request[tail] = x;
            pthread_mutex_unlock(&mutex);
            return b;
        }

        bool isEmpty()
        {
            return (tail == -1 && head == -1 )?true : false;
        }


        string pop()
        {
            pthread_mutex_lock(&mutex);

            string val;
            if(isEmpty())
            {
                cerr << "Queue is Empty!";
                pthread_mutex_unlock(&mutex);
                return val;
            }
            else if(head == tail){
                val = request[head];
                head = -1;
                tail  = -1;
            }
            else
            {
                val = request[head];
                head = ( head + 1 ) % size;
            }

            pthread_mutex_unlock(&mutex);
            return val;
        }

    bool isFull()
    {
        return ((tail + 1) %  size == head ) ? true:false;
    }


    ~myQueue()//destructor
        {
            delete [] request;
        }

};



#endif //HW5_MYQUEUE_H
