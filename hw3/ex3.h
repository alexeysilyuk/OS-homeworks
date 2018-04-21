//
// Created by alex on 4/10/18.
//

#ifndef HW3_EX3_H
#define HW3_EX3_H

#include <iostream>
#include <iomanip>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <vector>
#include <numeric>
#include <algorithm>
#include <unistd.h>


#include <ctime>
#include <vector>
#include <cstring>
#include <list>

#include "OrderBoard.h"
#include "MenuItem.h"
#include "RWLock.h"



using namespace std;


string ItemNameList[] =  {"Pizza","Salad","Hamburger","Spaghetti","Pie","Milkshake","Pelmeni","Salo","Borsh","Olivie"};
float ItemPrices[] = {10.00,7.50,12.00,9.00,9.50,6.00,8.00,3.99,8.10,5.30};
enum clientType {CUSTOMER,WAITER,PARENT};

enum ERROR_CODES {INCORRECT_PARAM=128,UNDUCCESS_FORK};
#define SEGSIZE 100*10 // 100 bytes for each item * maximum 10 items in menu


char  *itemsSegptr, *ordersSegptr,*earningsSegptr,*orderedAmountSegptr;
int* counter_cust_orders_segptr;
int counter_cust_shmid,ordersShmid,itemsShmid,earningsShmid,orderedAmountShmid;
int coutSem,waiterMutex,customersMutex;
int itemsSem,itemsReadSem,itemsWriteSem,itemsTryReadSem;
int ordersSem,ordersReadSem,ordersWriteSem,ordersTryReadSem;



vector<MenuItem> menuItems;
int simulationDuration,menuItemsCount;

struct timeval currentTime, startTime;
RWLock* itemsRWLock;
RWLock* ordersRWLock;


/// FUNCS Predeclarations
void manageCustomers(pid_t pid,int id);
void manageWaiters(pid_t pid,int id);
bool isTimedOut();
template<typename T> void allocateSharedMemory(pid_t pid,T** segptr, int *shmid);
void manageMainProcess(pid_t pid);
MenuItem randomChooseItem(vector<MenuItem> menu);
vector<MenuItem> itemsShmemToVector();
void ordersVectorToShmem(vector<OrderBoard> items,int ordersShmemID);
void updateOrderedAmount(vector<MenuItem> items, int itemID,int amount);
void itemsVectorToShmem(vector<MenuItem> items,int itemsShmid);
void initAllSemaphores();
void addOrderToBoard(int customerID, int itemID,int amount);
OrderBoard getOrderTokens(string tokenLine);
vector<OrderBoard> ordersShmemToVector();
void deleteSemsAndShmems();
////////////////////////////////


// bye bye banner at the end
void endBanner(){
    cout << R"(

 (`-').->(`-')  _ (`-')  _
 ( OO)_  ( OO).-/ ( OO).-/         .->        .->        .->
(_)--\_)(,------.(,------.     ,--.'  ,-.(`-')----. ,--.(,--.
/    _ / |  .---' |  .---'    (`-')'.'  /( OO).-.  '|  | |(`-')
\_..`--.(|  '--. (|  '--.     (OO \    / ( _) | |  ||  | |(OO )
.-._)   \|  .--'  |  .--'      |  /   /)  \|  |)|  ||  | | |  \
\       /|  `---. |  `---.     `-/   /`    '  '-'  '\  '-'(_ .',-.,-.,-.
 `-----' `------' `------'       `--'       `-----'  `-----'   '-''-''-'
)";
}

// check if all parameters are integers
bool analyzeParams(int argc, char * argv[]){
    for (int i=1;i<argc;i++)
    {
        try{
            stoi(argv[i]);
        }
        catch(exception ex)
        {
            cerr << "At least one argument has incorrect type\n"<<endl;
            exit(INCORRECT_PARAM);
        }
    }
    return true;
}

// chack allowed boundaries
bool analyzeParamsBounds(int time, int items, int customers,int waiters){
    if ((time <=30 && time >=0) &&
        (items >=0 && items<=10)&&
        (customers>=0 && customers<=10)&&
        (waiters>=0 && waiters<=3))
        return true;
    cerr << "One of params out of allowed boundaries\n";
    exit(INCORRECT_PARAM);


}

// generate menu, and add it to shared memory
void createMenuItems(int itemsAmount){

    for (int j = 0; j < itemsAmount; j++){
        menuItems.push_back(MenuItem(j,ItemNameList[j],ItemPrices[j]));

    }

    allocateSharedMemory(getpid(),&itemsSegptr,&itemsShmid);
    itemsVectorToShmem(menuItems,itemsShmid);

    cout << "============= Menu ============="<<endl;
    cout << "ID" <<setw(12)<<"Name"<<setprecision(5)<<setw(8)<<fixed<< setprecision(2)<<"Price" <<setw(8)<<"Orders" <<endl;
    for(vector<MenuItem>::iterator it = menuItems.begin();it!=menuItems.end();++it)
        cout << it->getID()+1<<setw(13)<< it->getName()<<setprecision(5)<<setw(8)<<fixed<< setprecision(2)<<it->getPrice() <<setw(5)<<it->getTotalOrdered() <<endl;

}

// return as string difference between initial and current time
string getTime(){

        gettimeofday(&currentTime,0);
        std::ostringstream os;
        float timestep = (currentTime.tv_sec - startTime.tv_sec) + (float)(currentTime.tv_usec - startTime.tv_usec)/1000000;
        os <<fixed<< setprecision(4) << timestep << "\t| ";

    return os.str();
}

// allocate shared memory for counter of all orders and earned money
void createEarningCounter(){
    allocateSharedMemory(getpid(),&earningsSegptr,&earningsShmid);
    strcpy(earningsSegptr,"0");

    allocateSharedMemory(getpid(),&orderedAmountSegptr,&orderedAmountShmid);
    strcpy(orderedAmountSegptr,"0");
}

// add earned money in current order to total
void increaseEarnings(float earnedNow){
    float earnedBefore = atof(earningsSegptr);
    earnedBefore+=earnedNow;
    char array[15];
    sprintf(array, "%f", earnedBefore);
    strcpy(earningsSegptr,array);

}

// add ordered amount to total
void increaseOrdersAmount(int orderedNow){
    int orderedBefore = atoi(orderedAmountSegptr);
    orderedBefore+=orderedNow;
    char array[15];
    sprintf(array, "%d", orderedBefore);
    strcpy(orderedAmountSegptr,array);
}

// creates order board for each customer in shared memory, fills with default values
void createOrdersBoards(int boardsAmount){

    allocateSharedMemory(getpid(),&ordersSegptr,&ordersShmid);

    vector<OrderBoard> orderBoard;
    for (int i =0; i< boardsAmount;i++)
        orderBoard.push_back(OrderBoard(i,-1,0,true));

    // not need here RWLocks, its only main process
    ordersVectorToShmem(orderBoard,ordersShmid);

    // initialize counters
    createEarningCounter();

}


// separate main process to needed amount of processes of waiter and customers
void startClientsProcesses(int clientsSize, int waitersSize){

    pid_t pid=-1;
    int myID=-1;
    clientType type=PARENT;

    for(int i=0;i<clientsSize;i++){   //create customers
        if((pid=fork())==0){
            myID = i;
             type = CUSTOMER;
            cout << getTime()<<"Customer " << myID <<": created PID " << getpid() << " PPID " << getppid() << endl;

            break;
        }
    }
    if(pid!=0)
        for(int i=0;i<waitersSize;i++){   //create waiters
            if((pid=fork())==0){
                myID = i;
                 type = WAITER;
                cout << getTime()<<"Waiter " << myID <<": created PID " << getpid() << " PPID " << getppid() << endl;

                break;
            }
        }

    if(pid ==-1)
    {
        cerr << "fork() error\n";
        exit(UNDUCCESS_FORK);
    }


    // for wach process run his fucntion detected by type
    if (type==WAITER)
        manageWaiters(getpid(),myID);
    else if (type == CUSTOMER)
        manageCustomers(getpid(),myID);
    else
    {
        int wpid,status=0;
        while ((wpid=wait(&status)) > 0);
        manageMainProcess(getpid());
    }

}


// create random number in given range using  "random_device" generator
// if your computer does't support this generator, use commented line placed after him
// return time in miliseconds
float getRandomSleepTime(int min,int max){
    std::random_device  gen;
//    std::mt19937 gen(time(0)+getpid());
    std::uniform_real_distribution<> uid(min, max);
    return uid(gen)*1000000; // for miliseconds
}

// generate random number for orders amount
int getRandNum(int min,int max){
    std::random_device  gen;
    std::uniform_int_distribution<> uid(min, max);
    return uid(gen);
}
// check if simulation time expired
bool isTimedOut(){
    return (simulationDuration<(currentTime.tv_sec-startTime.tv_sec));
}

// check if prevoius order been done
int isPrevOrderDone(int custID){

        // user read-write lock lo lock to reading from shared memory
        ordersRWLock->ReadLock();
        vector <OrderBoard> orders = ordersShmemToVector();
        ordersRWLock->ReadUnlock();

        // if customer order apears DONE, return 1
        for (vector<OrderBoard>::iterator it = orders.begin(); it != orders.end(); ++it)
            if (custID == it->getCustomerId()) {
                if (it->isDone() == true)
                    return 1;
                else
                    return 2;
            }
        orders.clear();

    return 3;
}

// manages customers processes
void manageCustomers(pid_t pid,int id){
    float sleepTime;

    while (!isTimedOut()){

        // sleep 3-6 seconds, float value
        sleepTime = getRandomSleepTime(3,6);
        usleep(sleepTime);

        // if this customer previous order been done
        if (isPrevOrderDone(id)==1){

                // get from shmem items menu
                itemsRWLock->ReadLock();
                vector<MenuItem> menu = itemsShmemToVector();
                itemsRWLock->ReadUnlock();

                // choose random item
                MenuItem itemToOrder = randomChooseItem(menu);

                // decide to order or not
                bool isOrdered = (getRandNum(0, 1) == 1) ? true : false;

                   // check again if prev order done and choosen to order
                if (isOrdered) {

                    p(coutSem);
                    cout << getTime() << "Customer " << id << ": reads menu about " << itemToOrder.getName();
                    int amount = getRandNum(1, 4);
                    cout << "( ordered, amount " << amount << " )\n";
                    v(coutSem);
                    sleep(1);

                    // get orders from shared memory
                    ordersRWLock->ReadLock();
                    vector<OrderBoard> ordersList = ordersShmemToVector();
                    ordersRWLock->ReadUnlock();

                    for(vector<OrderBoard>::iterator it = ordersList.begin();it!=ordersList.end();++it)
                        if(it->getCustomerId() == id)
                        {
                            it->setItemId(itemToOrder.getID());
                            it->setAmount(amount);
                            it->setDone(false);
                        }

                    // write updated orders board to shmem, and releas it
                    ordersVectorToShmem(ordersList,ordersShmid);


                }
                // if chosen not to order
                else {
                    p(coutSem);
                    cout << getTime() << "Customer " << id << ": reads menu about " << itemToOrder.getName();
                    cout << "( doesn't want to order )" << endl;
                    v(coutSem);
                    sleep(1);
                }
        }
    }

    // lock cout to print
    p(coutSem);
    cout << getTime()<<"Customer ID "<<id<< " PID "<<getpid()<<" end work PPID "<<getppid()<<endl;
    v(coutSem);

}

// manage waiters processes
void manageWaiters(pid_t pid,int waiterID){
    int sleepTime;
    while (!isTimedOut()){
        // sleep 1-2 seconds
        sleepTime = getRandomSleepTime(1,2);
        usleep(sleepTime);

        // only 1 waiter can take order same time
        p(waiterMutex);

        // lock reading from orders memory
        ordersRWLock->ReadLock();
        vector<OrderBoard> orders = ordersShmemToVector();
        ordersRWLock->ReadUnlock();


        for(vector<OrderBoard>::iterator it = orders.begin();it!=orders.end();++it)
        {
            // seek for unDONE order
            if(!it->isDone())
            {
                itemsRWLock->ReadLock();
                vector<MenuItem> menu =  itemsShmemToVector();
                itemsRWLock->ReadUnlock();

                // update ordered items in ITEMS memory
                updateOrderedAmount(menu,it->getItemId(),it->getAmount());
                menu.clear();

                it->setDone(true);  // set Done for order

                p(coutSem);
                cout << getTime()<<"Waiter ID "<<waiterID<< ": performes the order of customer "<<it->getCustomerId()<<" ("<<it->getAmount()<<" "<<ItemNameList[it->getItemId()]<<")\n";
                v(coutSem);

                // update orders board
                ordersVectorToShmem(orders,ordersShmid);

                // each waiter get one order same time
                break;
            }

        }
        // release semaphore
        v(waiterMutex);

        orders.clear();
    }

    p(coutSem);
    cout << getTime()<<"Waiter ID "<<waiterID<< " PID "<<getpid()<<" end work PPID "<<getppid()<<endl;
    v(coutSem);
}

// Parent main process management
void manageMainProcess(pid_t pid){

    menuItems.clear(); // remove old menu if exists

    menuItems = itemsShmemToVector();

    // print summary
    cout << "============= Menu ============="<<endl;
    cout << "ID" <<setw(12)<<"Name"<<setprecision(5)<<setw(8)<<fixed<< setprecision(2)<<"Price" <<setw(8)<<"Orders" <<endl;
    for(vector<MenuItem>::iterator it = menuItems.begin();it!=menuItems.end();++it)
        cout << it->getID()+1<<setw(13)<< it->getName()<<setprecision(5)<<setw(8)<<fixed<< setprecision(2)<<it->getPrice() <<setw(5)<<it->getTotalOrdered() <<endl;


    cout << "Total earned : "<<atof(earningsSegptr)<<" $\n";
    cout << "Total orders : "<<orderedAmountSegptr<<endl;
    cout << getTime() << "Main ID "<<getpid()<< " end work"<<endl;
    cout << getTime()<<"End of simulation\n";

    // print bye banner and reove shmems and semaphores
    endBanner();
    deleteSemsAndShmems();
}

// tamplate funtion to allocate shared memory
template<typename T>
void allocateSharedMemory(pid_t pid,T ** segptr, int *shmid){

    // most of this function taken from lab files, besides random
    int randNum = getRandNum(0,100000);
    key_t  key = ftok(".", randNum);

    if((*shmid = shmget(key, SEGSIZE, IPC_CREAT|IPC_EXCL|0666)) == -1)
    {
        printf("Shared memory segment exists - opening as client\n");

        /* Segment probably already exists - try as a client */

        if((*shmid = shmget(key, SEGSIZE, 0)) == -1)
        {
            perror("shmget Error : ");
            exit(1);
        }
    }
    /* Attach (map) the shared memory segment into the current process
*/
    if((*segptr = (T*)shmat(*shmid, 0, 0)) == NULL)
    {
        perror("shmat");
        exit(1);
    }
}


// gives command to remove shared memory and semaphores from OS
void deleteSemsAndShmems(){
shmctl(itemsShmid, IPC_RMID, 0);    // items shmem
shmctl(ordersShmid, IPC_RMID, 0);    // orders shmem
shmctl(orderedAmountShmid, IPC_RMID, 0);    // items ordered amount shmem
shmctl(earningsShmid, IPC_RMID, 0);    // total earnings shmem

shmctl(coutSem, IPC_RMID, 0);    // cout semaphore
shmctl(itemsSem, IPC_RMID, 0);    // items semaphore
shmctl(itemsReadSem, IPC_RMID, 0);    // itemsRead semaphore
shmctl(itemsWriteSem, IPC_RMID, 0);    // itemsWrite semaphore
shmctl(itemsTryReadSem, IPC_RMID, 0);    // itemsTryToRead semaphore
shmctl(ordersSem, IPC_RMID, 0);    // orders semaphore
shmctl(ordersReadSem, IPC_RMID, 0);    // ordersRead semaphore
shmctl(ordersWriteSem, IPC_RMID, 0);    // ordersWrite semaphore
shmctl(ordersTryReadSem, IPC_RMID, 0);    // ordersTryRead semaphore

shmctl(waiterMutex, IPC_RMID, 0);    // customers mutex semaphore
shmctl(customersMutex, IPC_RMID, 0);    // waiters mutex semaphore
}



//  chose random element from vector
MenuItem randomChooseItem(vector<MenuItem> menu)
{
    int randomItemIndex = getRandNum(0,menu.size()-1);
    return menu.at(randomItemIndex);

}


// receive line from shmem and sepearate it by '|' delimiter, and create MenuItem object
MenuItem getItemTokens(string tokenLine){
    list<string> tokens;
    std::stringstream ss(tokenLine);
    std::string token;

    while(std::getline(ss,token,'|'))
    {
        tokens.push_back(token);
    }

    int id = atoi(tokens.front().c_str());
    tokens.pop_front();
    string name = tokens.front();
    tokens.pop_front();
    float price = stof(tokens.front());
    tokens.pop_front();
    int orders = atoi(tokens.front().c_str());
    tokens.pop_front();

    return MenuItem(id, name, price, orders);
}

// receive line from shmem and sepearate it by '|' delimiter, and create OrderBoard object
OrderBoard getOrderTokens(string tokenLine){
    list<string> tokens;
    std::stringstream ss(tokenLine);
    std::string token;

    while(std::getline(ss,token,'|')){
        tokens.push_back(token);
    }

    int customerID = atoi(tokens.front().c_str());
    tokens.pop_front();
    int itemID = atoi(tokens.front().c_str());
    tokens.pop_front();

    int amount = atoi(tokens.front().c_str());
    tokens.pop_front();

    int done = atoi(tokens.front().c_str());
    tokens.pop_front();
    bool isDone = (done==1)? true : false;

    return OrderBoard(customerID,itemID,amount,isDone);
}


// for friendly use, get data from items shared memory and convert into vector array
vector<MenuItem> itemsShmemToVector(){

    vector<MenuItem> itemsList;

    std::stringstream ss(itemsSegptr);
    std::string token;
    if (itemsSegptr != NULL)
    {
        while(std::getline(ss,token,'\n')){
            MenuItem item = getItemTokens(token);
            itemsList.push_back(item);
        }
    }
    return itemsList;
}

// for friendly use, get data from orders shared memory and convert into vector array
vector<OrderBoard> ordersShmemToVector(){

    vector<OrderBoard> ordersList;

    std::stringstream ss(ordersSegptr);
    std::string token;
    if (ordersSegptr != NULL)
    {
        while(std::getline(ss,token,'\n')){
            OrderBoard item = getOrderTokens(token);
            ordersList.push_back(item);
        }
    }
    return ordersList;
}


//converts vector to char* and copy each element to pointer
void itemsVectorToShmem(vector<MenuItem> items,int itemsshmID){
//user reades-writers lock
    itemsRWLock->WriteLock();

    strcpy(itemsSegptr,""); //removes old
    for(vector<MenuItem>::iterator it = items.begin();it!=items.end();++it)
    {
        //write to shared memory
        strcat(itemsSegptr, it->to_string().c_str());
        strcat(itemsSegptr,"\n");
    }

    itemsRWLock->WriteUnlock();

}
// exactly same prev function fror orders
void ordersVectorToShmem(vector<OrderBoard> items,int ordersShmemID){

    ordersRWLock->WriteLock();
    strcpy(ordersSegptr,"");
    for(vector<OrderBoard>::iterator it = items.begin();it!=items.end();++it)
    {
        //write to shared memory
        strcat(ordersSegptr, it->to_string().c_str());
        strcat(ordersSegptr,"\n");
    }
    ordersRWLock->WriteUnlock();
}

// reveive vector with menu items, search item by id and update ordered amount
void updateOrderedAmount(vector<MenuItem> items, int itemID,int amount){

    for (vector<MenuItem>::iterator it = items.begin();it!=items.end();++it)
        if (it->getID()==itemID)
        {
            it->setTotalOrdered(it->getTotalOrdered()+amount);
            increaseEarnings(it->getPrice()*amount);
            increaseOrdersAmount(amount);
        }

    // uses RWlock for locking writing from readers
    itemsRWLock->WriteLock();
    itemsVectorToShmem(items,itemsShmid);
    itemsRWLock->WriteUnlock();
}

// initialize all semaphores
void initAllSemaphores(){


    // items read/write semaphores
    semkey   = ftok(".", getRandNum(0,100000));
    if((itemsSem=initsem(semkey,1))<0)
        exit(1);

    key_t semkey   = ftok(".", getRandNum(0,100000));
    if((itemsReadSem=initsem(semkey,1))<0)
        exit(1);

    semkey   = ftok(".", getRandNum(0,100000));
    if((itemsWriteSem=initsem(semkey,1))<0)
        exit(1);

    semkey   = ftok(".", getRandNum(0,100000));
    if((itemsTryReadSem=initsem(semkey,1))<0)
        exit(1);

    // orders read/write semaphores

    semkey   = ftok(".", getRandNum(0,100000));
    if((ordersSem=initsem(semkey,1))<0)
        exit(1);

    semkey   = ftok(".", getRandNum(0,100000));
    if((ordersReadSem=initsem(semkey,1))<0)
        exit(1);

    semkey   = ftok(".", getRandNum(0,100000));
    if((ordersWriteSem=initsem(semkey,1))<0)
        exit(1);

    semkey   = ftok(".", getRandNum(0,100000));
    if((ordersTryReadSem=initsem(semkey,1))<0)
        exit(1);

    // other smehpores

    // for cout
    semkey   = ftok(".", getRandNum(0,100000));
    if((coutSem=initsem(semkey,1))<0)
        exit(1);

    semkey   = ftok(".", getRandNum(0,100000));
    if((waiterMutex=initsem(semkey,1))<0)
        exit(1);

    semkey   = ftok(".", getRandNum(0,100000));
    if((customersMutex=initsem(semkey,1))<0)
        exit(1);


    // create global Locks for items and orders
    itemsRWLock = new RWLock(itemsReadSem,itemsWriteSem,itemsTryReadSem,itemsSem);
    ordersRWLock = new RWLock(ordersReadSem,ordersWriteSem,ordersTryReadSem,ordersSem);
}





#endif //HW3_EX3_H
