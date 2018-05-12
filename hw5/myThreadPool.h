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
 pthread_mutex_t mutexProducers,mutexConsumers,mutexDumper;


class myThreadPool{
private:

    int threadsAmount;
    pthread_t * pthreads;

public:

    myThreadPool(int threads)
    {
        threadsAmount=threads;

        pthread_mutex_init(&producer,NULL);
        pthread_mutex_init(&consumer,NULL);
        pthread_mutex_init(&mutexProducers,NULL);
        pthread_mutex_init(&mutexConsumers,NULL);
        pthread_mutex_init(&mutexDumper,NULL);

    }
    ~myThreadPool(){
        pthread_mutex_destroy(&producer);
        pthread_mutex_destroy(&consumer);
        pthread_mutex_destroy(&mutexProducers);
        pthread_mutex_destroy(&mutexConsumers);
        pthread_mutex_destroy(&mutexDumper);
         }

    // resolve ip for given domain name and return results
    static char ** resolveIP(string hostname, int* ips_found)
    {
        char **ipArray = new char*[MAX_IP_ADRESSES];
        const char* host = hostname.c_str();

        if(dnslookupAll(host, ipArray,MAX_IP_ADRESSES, ips_found) == UTIL_FAILURE)
            return NULL;
        else
            return ipArray;
    }

    //
    static void *putJob(void *filename)
    {

        char* file = (char*)filename;
        pthread_cond_t cond=PTHREAD_COND_INITIALIZER;
        string line;
        ifstream myfile;

        // each requester thread:
        //  1. open file for reading
        //  2. read line by line to get hostname
        //  3. try to push domain name to tasks queue
        //  4. wake consumer and wait till resolver thread will wake him up after processing
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
        pthread_exit(NULL);
    }

    // function for resolvers to get job from queue of tasks and process it
    static void *getJob(void *filename)
    {
        // run till all requesters finished to process all hosts  and queue is empty
        while(!allRequestersDone || !globalQueue->isEmpty())
        {
                pthread_mutex_lock(&mutexConsumers);
                pthread_mutex_lock(&consumer);

                // take hostname from queue
                request *result = globalQueue->pop();
                string host = result->hostname;

                pthread_mutex_unlock(&producer);    // unlock other requesters to add tasks

                pthread_cond_signal(result->taskDoneCond);  // signal to requester that added this hostname about finishing process
                pthread_mutex_unlock(&mutexConsumers);


                int ips_found = 0;
                string ip_all;
                char **ip = resolveIP(host, &ips_found);    // get all ip adresses for given hostname
                if (ip != NULL)
                {
                    for (int i = 0; i < ips_found; i++) {
                        ip_all += ip[i];
                        ip_all += ",";
                    }
                    ip_all = ip_all.substr(0, ip_all.size() - 1);
                    resultsArray->addTail(host, ip_all);
                }
                else
                {
                    cerr << host << " : IP address for domain not found" << endl;
                    resultsArray->addTail(host, "");
                }
        }

        pthread_exit(NULL);
    }

    // create requesters threads to process files
    static void* AquireRequests(void* args){

        char* argv[globalargc-2];
        char **new_chars=reinterpret_cast<char**>(args);

        // copy only filenames from argv to new array
        for (int i=1,j=0;i<globalargc-1;i++,j++)
        {
            argv[j]= strdup(new_chars[i]);
        }

        //make pthreads joinable
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        // create threads, one for each file
        pthread_t workingThreads[totalFiles];

        int rc;
        // creating requests threads
        for(int i = 0; i < totalFiles; i++)
        {
            rc = pthread_create(&workingThreads[i],&attr, putJob, argv[i]);
            if(rc)
            {
                cerr <<"ERROR in pthread_create() :"+rc<<endl;
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
                cerr<<"ERROR in pthread_join() : "<< rc <<endl;
                exit(PTHREAD_JOIN_ERROR);
            }
        }

        cout << blueB << "Requesters threads finished"<<defB<<endl;
        allRequestersDone=true;//say to resolvers that there aro no more new task will be added
        pthread_exit(NULL);
    }

    // create resolvers threads
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
                cerr <<"ERROR in pthread_create() :"+rc<<endl;
                exit(PTHREAD_CREATE_ERROR);
            }
        }

        void* status;

        for(int i = 0; i < MAX_RESOLVER_THREADS; i++)
        {
            rc = pthread_join(workingThreads[i], &status);
            if (rc)
            {
                cerr<<"ERROR in pthread_join() : "<< rc <<endl;
                exit(PTHREAD_JOIN_ERROR);
            }

        }
        cout << blueB << "Resolvers threads finished"<<defB<<endl;

        pthread_exit(NULL);
    }

    // get jobs from result array to print to file
    static void *getDumperJob(void * _)
    {
        // run till results array is empty
            while(!resultsArray->isEmpty()) {
                pthread_mutex_lock(&mutexDumper);
                // double check to prevent double pop
                if(!resultsArray->isEmpty())
                {
                    // get line from results array and print in to file
                    string res = resultsArray->popFirst();
                    if(res.compare("NULL")!=0)
                    {
                        resultsArray->writeToFile(res);
                    }
                }
                pthread_mutex_unlock(&mutexDumper);
        }
        pthread_exit(NULL);
    }

    //create dumpers threads
    static void* DumpDesults(void* args){

        pthread_t workingThreads[DUMPER_THREADS];

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        int rc;
        for(int i = 0; i < DUMPER_THREADS; i++)
        {
            rc = pthread_create(&workingThreads[i],&attr, getDumperJob, &i);
            if(rc)
            {
                cerr <<"ERROR in pthread_create() :"+rc<<endl;
                exit(PTHREAD_CREATE_ERROR);
            }
        }
        void* status;
        for(int i = 0; i < DUMPER_THREADS; i++)
        {
            rc = pthread_join(workingThreads[i], &status);
            if (rc)
            {
                cerr<<"ERROR in pthread_join() : "<< rc <<endl;
                exit(PTHREAD_JOIN_ERROR);
            }

        }
        cout << blueB << "All dumpers threads finished"<<defB<<endl;

        pthread_exit(NULL);

    }
};

#endif //HW5_MYTHREADPOOL_H
