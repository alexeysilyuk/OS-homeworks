

#include "ex3.h"



using namespace std;



int main(int argc, char* argv[]) {

    if (argc!=5){
        cout << " Incorrect amount of parameters\n";
        exit(INCORRECT_PARAM);
    }
    else
        analyzeParams(argc,argv);// check correct type of params,all must be integers

    // get values of args
    simulationDuration = stoi(argv[1]);
    menuItemsCount = stoi(argv[2]);
    int customersCount=stoi(argv[3]);
    int waitersCount = stoi(argv[4]);

    // check received parameters boundaries
    analyzeParamsBounds(simulationDuration,menuItemsCount,customersCount,waitersCount);


    cout << "===== Simulation arguments =====" << endl;
    cout << "Simulation time: "<<simulationDuration<<endl;
    cout << "Menu items count: "<<menuItemsCount<<endl;
    cout << "Customers count: "<<customersCount<<endl;
    cout << "Waiters count: "<< waitersCount<<endl;
    cout << "================================"<<endl;

    initAllSemaphores();
    createMenuItems(menuItemsCount);
    //initAllSemaphores();
    cout << "================================"<<endl;

    gettimeofday (&startTime, NULL); // start counting time

    cout <<getTime()<< "Main process ID " << getpid() << " start" << endl;
    cout << "================================"<<endl;


    createOrdersBoards(customersCount);

    startClientsProcesses(customersCount,waitersCount);



    // free allocated memory
//    FreeMenuItemsMemory(menuItemsCount);
    return 0;
}


