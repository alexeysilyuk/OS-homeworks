//
// Created by alex on 5/3/18.
//

#ifndef HW5_MYTHREADPOOL_H
#define HW5_MYTHREADPOOL_H

#include "myQueue.h"
#include "globalQueue.h"
#include <pthread.h>
#include <fstream>
#include <sys/time.h>
#include <algorithm>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
extern "C"
{
#include "libmyutil/util.h"
}
int coutSem;
int requests=0;
class myThreadPool{
private:

    int threadsAmount;
    pthread_mutex_t producer,consumer;

public:

    myThreadPool(int threads)
    {
        threadsAmount=threads;
        pthread_mutex_init(&producer,NULL);
        pthread_mutex_init(&consumer,NULL);

        key_t semkey   = ftok(".", getpid());
        if((coutSem=initsem(semkey,1))<0)
            exit(1);
    }
    ~myThreadPool(){
        shmctl(coutSem, IPC_RMID, 0);    // cout semaphore
         }

    static string resolveIP(string hostname){
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

    static void *putJob(void *filename)
    {
        char* file = (char*)filename;
        string line;
        ifstream myfile;
        myfile.open(file);
        while (getline(myfile, line))
        {
            if(!resultsArray->contains(line)) {
                while (globalQueue->push(line) == false) {
                    std::random_device gen;
                    std::uniform_real_distribution<> uid(0, 100);
                    usleep(uid(gen)); // for miliseconds
                }
            }
        }
//        cout << "finished file : "<<file<<endl;
        myfile.close();
        pthread_exit(NULL);
    }


    static void *getJob(void *filename)
    {
        while(true)
        {
            if(!globalQueue->isEmpty()) {
                string host = globalQueue->pop();
                string ip = resolveIP(host);
                resultsArray->addTail(host,ip);
                p(coutSem);
                cout<<host<<" : " << ip << endl;
                v(coutSem);
            }
            else
            {
//                cout << "no active requests, goint to sleep...\n";
                sleep(1);
//                std::random_device  gen;
//                std::uniform_real_distribution<> uid(0, 100);
//                usleep(uid(gen)); // for miliseconds
            }
        }
//        char* file = (char*)filename;
//        string line;
//        ifstream myfile;
//        myfile.open(file);
//        while (getline(myfile, line))
//        {
//            while(globalQueue->pop()==false)
//            {
//                std::random_device  gen;
//                std::uniform_real_distribution<> uid(0, 100);
//                usleep(uid(gen)); // for miliseconds
//            }
//        }
//        myfile.close();

        pthread_exit(NULL);
    }

    static void* AquireRequests(void* args){
        char* argv[globalargc-2];
        char **new_chars=reinterpret_cast<char**>(args);
        for (int i=1,j=0;i<globalargc-1;i++,j++)
        {
            argv[j]= strdup(new_chars[i]);
//            cout << argv[i]<<endl;
        }


        pthread_t workingThreads[globalargc-2];

        int rc;
        // creating requests threads
        for(int i = 0; i < globalargc-1; i++)
        {

            rc = pthread_create(&workingThreads[i],NULL, putJob, argv[i]);
            if(rc)
            {
                cout <<"creating Request thread failed! Error code returned is:"+rc<<endl;
                exit(-1);
            }
        }
        pthread_exit(NULL);
    }


    static void* ServeRequest(void* args){
//        argvc* arg  = (argvc*)args;

        pthread_t workingThreads[MAX_RESOLVER_THREADS];

        int rc;
        // creating requests threads
        for(int i = 0; i < MAX_RESOLVER_THREADS; i++)
        {
            rc = pthread_create(&workingThreads[i],NULL, getJob, &i);
            if(rc)
            {
                cout <<"creating Request thread failed! Error code returned is:"+rc<<endl;
                exit(-1);
            }
        }
        pthread_exit(NULL);
    }


};

#endif //HW5_MYTHREADPOOL_H
