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

#define MAX_IP_ADRESSES 10
extern "C"
{
#include "libmyutil/util.h"
}
int coutSem;
int requests=0;
pthread_mutex_t producer,consumer,mutex;
pthread_cond_t requester,resolver;


class myThreadPool{
private:

    int threadsAmount;

public:

    myThreadPool(int threads)
    {
        threadsAmount=threads;

        pthread_mutex_init(&producer,NULL);
        pthread_mutex_init(&consumer,NULL);
        pthread_mutex_init(&mutex,NULL);
        pthread_cond_init (&requester, NULL);
        pthread_cond_init (&resolver, NULL);


        key_t semkey   = ftok(".", getpid());
        if((coutSem=initsem(semkey,1))<0)
            exit(1);
    }
    ~myThreadPool(){
        shmctl(coutSem, IPC_RMID, 0);    // cout semaphore
         }

    static char ** resolveIP(string hostname,int * ips_found){
        char **ipArray = new char*[MAX_IP_ADRESSES];
        const char* host = hostname.c_str();

        if(dnslookupAll(host, ipArray,MAX_IP_ADRESSES, ips_found) == UTIL_FAILURE)
        {
            return NULL;
        }
        else
        {

            return ipArray;
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
                if(globalQueue->push(line) == false) {
                    {
                        pthread_mutex_lock(&producer);
                        pthread_cond_wait(&requester,&producer);
                        globalQueue->push(line);
                        pthread_mutex_unlock(&producer);
                    }
                } else
                    pthread_cond_signal(&resolver);
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
                pthread_mutex_lock(&consumer);
                string host = globalQueue->pop();
                pthread_cond_signal(&requester);

                int ips_found=0;
                string ip_all;
                char** ip = resolveIP(host,&ips_found);
                if(ip!=NULL)
                {
                    for(int i=0;i<ips_found;i++)
                    {
                        ip_all+=ip[i];
                        ip_all+=",";
                    }
                    ip_all=ip_all.substr(0,ip_all.size()-1);
                    resultsArray->addTail(host, ip_all);
                }
                else
                {
                    resultsArray->addTail(host, "");
                }


            }
            else
            {
                pthread_cond_wait(&resolver,&consumer);
            }
            pthread_mutex_unlock(&consumer);
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
