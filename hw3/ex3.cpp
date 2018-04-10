
#include "MenuItem.h"
#include "ex3.h"
#include <unistd.h>


using namespace std;



int main(int argc, char* argv[]) {
    gettimeofday (&startTime, NULL);
    if (argc!=5){
        cout << " Incorrect amount of parameters\n";
        exit(INCORRECT_PARAM);
    }
    else
        analyzeParams(argc,argv);// check correct type of params,all must be integers

    // get values of args
    int simulationTime = stoi(argv[1]);
    int menuItemsCount = stoi(argv[2]);
    int customesCount=stoi(argv[3]);
    int waitersCount = stoi(argv[4]);

    // check received parameters boundaries
    analyzeParamsBounds(simulationTime,menuItemsCount,customesCount,waitersCount);


    cout << "===== Simulation arguments =====" << endl;
    cout << "Simulation time: "<<simulationTime<<endl;
    cout << "Menu items count: "<<menuItemsCount<<endl;
    cout << "Customers count: "<<customesCount<<endl;
    cout << "Waiters count: "<< waitersCount<<endl;
    cout << "================================"<<endl;


    createMenuItems(menuItemsCount);
    cout << "================================"<<endl;

sleep(2);
    cout <<getTime()<< "Main process ID " << getpid() << " start" << endl;
    cout << "================================"<<endl;





    // free allocated memory
    FreeMenuItemsMemory(menuItemsCount);
    return 0;
}


