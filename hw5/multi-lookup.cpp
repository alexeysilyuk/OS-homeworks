#include <iostream>
#include <pthread.h>
#include "multi-lookup.h"
#include "myQueue.h"
#include "myThreadPool.h"
#include "globalQueue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <random>
#include <iostream>


using namespace std;
//////////////////////////////////////////////////////////////////////////////////////////////////
// names : redC,redF,defF,defB,greF,greB,blueB,blueF are used for colorful printing to console  //
//////////////////////////////////////////////////////////////////////////////////////////////////

void banner(){
    cout<<greF<< R"(
================================================================================================================
 /$$$$$$$  /$$   /$$  /$$$$$$                                                  /$$
| $$__  $$| $$$ | $$ /$$__  $$                                                | $$
| $$  \ $$| $$$$| $$| $$  \__/          /$$$$$$   /$$$$$$   /$$$$$$$  /$$$$$$ | $$ /$$    /$$ /$$$$$$   /$$$$$$
| $$  | $$| $$ $$ $$|  $$$$$$  /$$$$$$ /$$__  $$ /$$__  $$ /$$_____/ /$$__  $$| $$|  $$  /$$//$$__  $$ /$$__  $$
| $$  | $$| $$  $$$$ \____  $$|______/| $$  \__/| $$$$$$$$|  $$$$$$ | $$  \ $$| $$ \  $$/$$/| $$$$$$$$| $$  \__/
| $$  | $$| $$\  $$$ /$$  \ $$        | $$      | $$_____/ \____  $$| $$  | $$| $$  \  $$$/ | $$_____/| $$
| $$$$$$$/| $$ \  $$|  $$$$$$/        | $$      |  $$$$$$$ /$$$$$$$/|  $$$$$$/| $$   \  $/  |  $$$$$$$| $$
|_______/ |__/  \__/ \______/         |__/       \_______/|_______/  \______/ |__/    \_/    \_______/|__/

================================================================================================================)"<<defF<<"\n\n";

}

void endbanner(){
    cout <<greF<< R"(
=======================================================================
                                       ,---.o     o     |             |
,---.,---.,---.,---.,---.,---.,-.-.    |__. .,---..,---.|---.,---.,---|
|   ||    |   ||   ||    ,---|| | |    |    ||   ||`---.|   ||---'|   |
|---'`    `---'`---|`    `---^` ' '    `    ``   '``---'`   '`---'`---'
|              `---'

)"<<defF;

}


int main(int argc, char* argv[]) {
    banner();

    // check amount of total args, 1 for program, 2 for input files, 1 for output
    if(argc<3){
        cerr << redF<< "Incorrect amount of parameters, minimum 2, 1 for input file and 1 for output\n"<<defF;
        exit(INCORRECT_PARAMS_AMOUNT);
    }


    cout <<"Files received to process: "<<argc-2<<endl;
    cout << "--------------------------------------\n";
    for(int i=1;i<argc-1;i++){

        // try to open received files to check if exists
        FILE *file=fopen(argv[i],"r");
        if(file==NULL)
        {
            cerr << redF<< "can't open file to read \""<<argv[i]<<"\""<<defF<<endl;
            exit(IMPORT_FILE_READ_ERROR);
        }
        else
        {
            // print file name
            cout <<i<<": \""<<argv[i]<<"\""<<endl;
            fclose(file);
        }
    }
    cout << "--------------------------------------\n";

    // check at least 2 input files
    if(argc-2>=1)
    {
        totalFiles=argc-2;  // total files amount
        globalargc=argc;    // global argc value
        pthread_t pc[2];    // array of threads for resolvers and requesters threadpools
        int rc,i=0;
        void* status;

        resultsArray->setOutputFileName(argv[argc-1]);  // set output file name to resultsArray

        // create thread pools
        myThreadPool *requestsPool = new myThreadPool(totalFiles);
        myThreadPool *resolversPool = new myThreadPool(MAX_RESOLVER_THREADS); // MAX_RESOLVER_THREADS declared in 'globalQueue.h'
        myThreadPool *dumperPool = new myThreadPool(DUMPER_THREADS); // DUMPER_THREADS declared in 'globalQueue.h'

        // create attr to make pthreads joinable
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        // create one thread for requesters and one thread for resolvers


        // create thread for requesters threadpool
        cout << blueB<<"Creating Requesters threads ..."<<defB<<endl;
            rc = pthread_create(&pc[0], &attr,requestsPool->AquireRequests , argv);
            if (rc) {
                cerr <<  redF<< "ERROR: Requesters pthread_create():" + rc <<defF<< endl;
                exit(PTHREAD_CREATE_ERROR);
                }

        // create thread for resolvers threadpool
        cout << blueB<< "Creating Resolvers threads ..."<<defB<<endl;
            rc = pthread_create(&pc[1], &attr,resolversPool->ServeRequest , &i);
            if (rc) {
                cerr << redF<< "ERROR: Resolvers pthread_create():" + rc <<defF<< endl;
                exit(PTHREAD_CREATE_ERROR);
            }

        // wait for all requesters and resolvers to finish
        for(int i = 0; i < 2; i++)
        {
            rc = pthread_join(pc[i], &status);
            if (rc)
            {
                cerr<<redF<<"ERROR in pthread_join() : "<< rc <<defF<<endl;
                exit(PTHREAD_JOIN_ERROR);
            }
        }

        // create timpers thread
        cout << blueB<< "Creating dumpers threads ."<<defB<<endl;

        pthread_t dumperThread;
        rc = pthread_create(&dumperThread, NULL,dumperPool->DumpDesults, &i);
        if (rc) {
            cerr << redF<< "ERROR: Dumpers pthread_create():" + rc <<defF<< endl;
            exit(PTHREAD_CREATE_ERROR);
        }

        // wait for dumper thread finish
        rc = pthread_join(dumperThread, &status);
        if (rc)
        {
            cerr<<redF<<"ERROR in pthread_join() : "<< rc <<defF<<endl;
            exit(PTHREAD_JOIN_ERROR);
        }
        cout << blueB<< "Dumpers threads finished"<<defB<<endl;



        endbanner();
        pthread_exit(NULL); //dont finish main thread before other threads
    }

    return 0;
}