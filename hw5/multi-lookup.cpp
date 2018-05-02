#include <iostream>
#include <pthread.h>
#include "multi-lookup.h"
#include "myQueue.h"
#include <sys/ipc.h>
#include "Semaphore.h"
#include <sys/shm.h>
extern "C"
{
#include "libmyutil/util.h"
}


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <unistd.h>
#include <random>

myQueue requestQueue = myQueue(16);

#define NUM_THREADS	10

using namespace std;
int coutSem;





string resolveIP(string hostname){
    char ipstr[1024];
    const char* host = hostname.c_str();

    if(dnslookup(host, ipstr, sizeof(ipstr)) == UTIL_FAILURE)
    {
        return "Error";
    }
    else
    {
        return ipstr;
    }
}

float getRandomSleepTime(int min,int max){
    std::random_device  gen;
//    std::mt19937 gen(time(0)+getpid());
    std::uniform_real_distribution<> uid(min, max);
    return uid(gen); // for miliseconds
}

void *takeJob(void *threadid)
{

    int id = *(int*)threadid;
    while(!requestQueue.isEmpty())
    {
        string host=requestQueue.pop();
        p(coutSem);
        cout<< "pid:"<<id<<" - "<<host<< " : "<<resolveIP(host)<<endl;
        v(coutSem);

        usleep(getRandomSleepTime(0,100));
    }

    pthread_exit(NULL);
}

void work(){
    pthread_t threads[MAX_RESOLVER_THREADS];
    int rc;
    int i;

    for (i = 0; i < MAX_RESOLVER_THREADS; i++)
    {
        rc = pthread_create(&threads[i], NULL, takeJob, &i);

        if (rc)
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    pthread_exit(NULL);
}


int main(int argc, char* argv[]) {

    if(argc<2){
        cerr << "Incorrect amount of parameters, minimum 2, 1 for input file and 1 for output\n";
        exit(INCORRECT_PARAMS_AMOUNT);
    }

    key_t semkey   = ftok(".", getpid());
    if((coutSem=initsem(semkey,1))<0)
        exit(1);


    for (int i=1; i<argc-1;i++) {
        string line;
        ifstream myfile;
        myfile.open(argv[i]);
        while (getline(myfile, line))
        {
            requestQueue.push(line);
        }
        myfile.close();
    }

//    while(!requestQueue.isEmpty())
//        cout << requestQueue.pop()<<endl;
    work();

    shmctl(coutSem, IPC_RMID, 0);    // cout semaphore
    return 0;
}