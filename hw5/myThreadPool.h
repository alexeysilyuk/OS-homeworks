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

 pthread_mutex_t producer,consumer,mutex;
 pthread_mutex_t mutexProducers,mutexConsumers;

class myThreadPool{
private:

    int threadsAmount;

public:

    myThreadPool(int threads)
    {
        threadsAmount=threads;

        pthread_mutex_init(&producer,NULL);
        pthread_mutex_init(&consumer,NULL);
        pthread_mutex_init(&mutexProducers,NULL);
        pthread_mutex_init(&mutexConsumers,NULL);


    }
    ~myThreadPool(){
        pthread_mutex_destroy(&producer);
        pthread_mutex_destroy(&consumer);
        pthread_mutex_destroy(&mutexProducers);
        pthread_mutex_destroy(&mutexConsumers);
         }

    static char ** resolveIP(string hostname, int* ips_found)
    {
        char **ipArray = new char*[MAX_IP_ADRESSES];
        const char* host = hostname.c_str();

        if(dnslookupAll(host, ipArray,MAX_IP_ADRESSES, ips_found) == UTIL_FAILURE)
            return NULL;
        else
            return ipArray;
    }

    static void *putJob(void *filename)
    {
        char* file = (char*)filename;
        pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
        pthread_mutex_t resultMutex;
        string line;
        ifstream myfile;

        myfile.open(file);

        while (getline(myfile, line))
        {
            if(!resultsArray->contains(line)) {

                pthread_mutex_lock(&mutexProducers);
                pthread_mutex_lock(&producer);
                globalQueue->push(line,&cond,&mutexProducers);

                pthread_mutex_unlock(&consumer);
                pthread_cond_wait(&cond,&mutexProducers);

                pthread_mutex_unlock(&mutexProducers);
                }
            }
        myfile.close();



        cout << "All domains from file \""<< (char*)filename<< "\" are processed!\n";
        pthread_exit(NULL);
    }


    static void *getJob(void *filename)
    {
        while(!(allRequestersDone==true && globalQueue->isEmpty()))
        {

                pthread_mutex_lock(&mutexConsumers);
                pthread_mutex_lock(&consumer);
                string host = globalQueue->pop();

                pthread_mutex_unlock(&producer);
                pthread_mutex_unlock(&mutexConsumers);

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

        }
        pthread_exit(NULL);
    }

    static void* AquireRequests(void* args){
        char* argv[globalargc-2];
        char **new_chars=reinterpret_cast<char**>(args);

        // copy only filenames
        for (int i=1,j=0;i<globalargc-1;i++,j++)
        {
            argv[j]= strdup(new_chars[i]);
        }

        //make pthreads joinable
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        pthread_t workingThreads[totalFiles];

        int rc;
        // creating requests threads
        for(int i = 0; i < totalFiles; i++)
        {
            rc = pthread_create(&workingThreads[i],&attr, putJob, argv[i]);
            if(rc)
            {
                cout <<"ERROR in pthread_create() :"+rc<<endl;
                exit(PTHREAD_CREATE_ERROR);
            }
        }

        void *status;
        // waiting to finishing all requests threads
        for(int i = 0; i < totalFiles; i++)
        {
            rc = pthread_join(workingThreads[i], &status);
            if (rc)
            {
                cout<<"ERROR in pthread_join() : "<< rc <<endl;
                exit(PTHREAD_JOIN_ERROR);
            }
        }

        cout << "All requesters threads finished\n";
        allRequestersDone=true;//say to resolvers that there aro no more new task will bee added
        pthread_exit(NULL);
    }


    static void* ServeRequest(void* args){

        pthread_t workingThreads[MAX_RESOLVER_THREADS];

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        int rc;
        // creating requests threads
        for(int i = 0; i < MAX_RESOLVER_THREADS; i++)
        {
            rc = pthread_create(&workingThreads[i],&attr, getJob, &i);
            if(rc)
            {
                cout <<"ERROR in pthread_create() :"+rc<<endl;
                exit(PTHREAD_CREATE_ERROR);
            }
        }

        void* status;

        for(int i = 0; i < MAX_RESOLVER_THREADS; i++)
        {
            rc = pthread_join(workingThreads[i], &status);
            if (rc)
            {
                cout<<"ERROR in pthread_join() : "<< rc <<endl;
                exit(PTHREAD_JOIN_ERROR);
            }

        }
        cout << "All resolvers threads finished\n";

        pthread_exit(NULL);
    }
};

#endif //HW5_MYTHREADPOOL_H
