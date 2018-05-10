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

    cout <<"files received: "<<argc-2<<endl;
    cout << "--------------------------------------\n";
    for(int i=1;i<argc-1;i++){

        FILE *file=fopen(argv[i],"r");
        if(file==NULL)
        {
            cerr << "can't open file to read \""<<argv[i]<<"\""<<endl;
            exit(IMPORT_FILE_READ_ERROR);
        } else
            cout <<i<<": \""<<argv[i]<<"\""<<endl;
    }
    cout << "--------------------------------------\n";
    if(argc-2>=1)
    {
        totalFiles=argc-2;


        resultsArray->setOutputFileName(argv[argc-1]);
        myThreadPool *requestsPool = new myThreadPool(totalFiles);
        myThreadPool *resolversPool = new myThreadPool(10);


        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        // create one thread for requesters and one thread for resolvers
        globalargc=argc;
        pthread_t pc[2];
        int rc;

                rc = pthread_create(&pc[0], &attr,requestsPool->AquireRequests , argv);
                if (rc) {
                    cout << "creating Request thread failed! Error code returned is:" + rc << endl;
                    exit(PTHREAD_CREATE_ERROR);
                }

            int i;
                rc = pthread_create(&pc[1], NULL,resolversPool->ServeRequest , &i);
                if (rc) {
                    cout << "creating Request thread failed! Error code returned is:" + rc << endl;
                    exit(PTHREAD_CREATE_ERROR);
                }

        void* status;
        for(int i = 0; i < 2; i++)
        {
            rc = pthread_join(pc[i], &status);
            if (rc)
            {
                cout<<"ERROR in pthread_join() : "<< rc <<endl;
                exit(PTHREAD_JOIN_ERROR);
            }
        }

        cout <<"END"<<endl;
        pthread_exit(NULL);
    }

    return 0;
}