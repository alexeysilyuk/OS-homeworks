#include <iostream>
#include <pthread.h>
#include "multi-lookup.h"
#include "myQueue.h"

#include "Semaphore.h"
#include "myThreadPool.h"

#include "globalQueue.h"

#define QUEUE_SIZE 30

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <random>



using namespace std;


int main(int argc, char* argv[]) {

    if(argc<2){
        cerr << "Incorrect amount of parameters, minimum 2, 1 for input file and 1 for output\n";
        exit(INCORRECT_PARAMS_AMOUNT);
    }

    int filesAmount;

    cout <<argc<<endl;
    for(int i=0;i<argc;i++){
        cout << "argc: "<<i<<", argv["<<i<<"] : "<<argv[i]<<endl;
    }

    if(argc-2>=1)
    {
        filesAmount=argc-2;
        myThreadPool *requestsPool = new myThreadPool(filesAmount);
        myThreadPool *resolversPool = new myThreadPool(10);

        globalargc=argc;
        pthread_t pc[2];
        int rc;

                rc = pthread_create(&pc[0], NULL,requestsPool->AquireRequests , argv);
                if (rc) {
                    cout << "creating Request thread failed! Error code returned is:" + rc << endl;
                    exit(-1);
                }

            int i;
                rc = pthread_create(&pc[1], NULL,resolversPool->ServeRequest , &i);
                if (rc) {
                    cout << "creating Request thread failed! Error code returned is:" + rc << endl;
                    exit(-1);
                }
        void *status;
        pthread_exit(NULL);
//        for(int i=0; i < 2; i++) {
//            rc = pthread_join(pc[i], &status);
//            if (rc) {
//                cout << "ERROR; return code from pthread_join() is " << rc << endl;
//                exit(-1);
//            }
//        }
    }








//    while(!requestQueue.isEmpty())
//        cout << requestQueue.pop()<<endl;


    return 0;
}