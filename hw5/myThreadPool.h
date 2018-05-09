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
pthread_mutex_t finished;

class myThreadPool{
private:

    int threadsAmount;


public:

    myThreadPool(int threads)
    {
        threadsAmount=threads;

        pthread_mutex_init(&finished,NULL);
//        pthread_mutex_init(&producer,NULL);
//        pthread_mutex_init(&consumer,NULL);
//        pthread_mutex_init(&mutex,NULL);
//        pthread_cond_init (&requester, NULL);
//        pthread_cond_init (&resolver, NULL);


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
                globalQueue->push(line);
            }
        }

        myfile.close();

        myfile.open(file);
            while (getline(myfile, line))
            {
                while (!resultsArray->contains(line));
            }
        myfile.close();

        pthread_mutex_lock(&finished);
        finishedFiles++;
        pthread_mutex_unlock(&finished);

//return 0;
        pthread_exit(NULL);
    }


    static void *getJob(void *filename)
    {
        while(true)
        {

            pthread_mutex_lock(&finished);
            if(finishedFiles==totalFiles)
            {
                cout <<finishedFiles<<" / "<<totalFiles<<endl;
                pthread_mutex_unlock(&finished);
                break;
            }
            pthread_mutex_unlock(&finished);
                string host = globalQueue->pop();
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
                    cerr <<host<< " : IP address for domain not found"<<endl;
                    resultsArray->addTail(host, "");
                }
        cout << pthread_self()<<endl;
        }

        pthread_exit(NULL);
    }

    static void* AquireRequests(void* args){
        char* argv[globalargc-2];
        char **new_chars=reinterpret_cast<char**>(args);
        for (int i=1,j=0;i<globalargc-1;i++,j++)
        {
            argv[j]= strdup(new_chars[i]);
        }

        pthread_t workingThreads[totalFiles];

        int rc;
        // creating requests threads
        for(int i = 0; i < totalFiles; i++)
        {
            rc = pthread_create(&workingThreads[i],NULL, putJob, argv[i]);
            if(rc)
            {
                cout <<"ERROR in pthread_create() :"+rc<<endl;
                exit(PTHREAD_CREATE_ERROR);
            }
        }

        void *status;

//        for(int i = 0; i < totalFiles; i++)
//        {
//            rc = pthread_join(workingThreads[i], &status);
//            if (rc)
//            {
//                cout<<"ERROR in pthread_join() : "<< rc <<endl;
//                exit(PTHREAD_JOIN_ERROR);
//            }
//        }

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
                cout <<"ERROR in pthread_create() :"+rc<<endl;
                exit(PTHREAD_CREATE_ERROR);
            }
        }

        void* status;

//        for(int i = 0; i < MAX_RESOLVER_THREADS; i++)
//        {
//            rc = pthread_join(workingThreads[i], &status);
//            if (rc)
//            {
//                cout<<"ERROR in pthread_join() : "<< rc <<endl;
//                exit(PTHREAD_JOIN_ERROR);
//            }
//        }


        pthread_exit(NULL);
    }


};

#endif //HW5_MYTHREADPOOL_H
