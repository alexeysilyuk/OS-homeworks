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
#include "Semaphore.h"

using namespace std;


string ItemNameList[] =  {"Pizza","Salad","Hamburger","Spaghetti","Pie","Milkshake","Pelmeni","Salo","Borsh","Olivie"};
float ItemPrices[] = {10.00,7.50,12.00,9.00,9.50,6.00,8.00,3.99,8.10,5.30};
enum clientType {CUSTOMER,WAITER,PARENT};

enum ERROR_CODES {INCORRECT_PARAM=128};
#define SEGSIZE 100*10 // 100 bytes for each item * maximum 10 items in menu


char  *itemsSegptr, *ordersSegptr,*earningsSegptr,*orderedAmountSegptr;
int ordersShmid,itemsShmid,earningsShmid,orderedAmountShmid;
int itemsSem,ordersSem,waitersSem,customersSem,coutSem, reader,writer;


vector<MenuItem> menuItems;
int simulationDuration,menuItemsCount;

struct timeval currentTime, startTime;


/// FUNCS Predeclarations
void manageCustomers(pid_t pid,int id);
void manageWaiters(pid_t pid,int id);
bool isTimedOut();
void allocateSharedMemory(pid_t pid,char** segptr, int *shmid);
void printMenu(char* segptr);
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


bool analyzeParamsBounds(int time, int items, int customers,int waiters){
    if ((time <=30 && time >=0) &&
        (items >=0 && items<=10)&&
        (customers>=0 && customers<=10)&&
        (waiters>=0 && waiters<=3))
        return true;
    cerr << "One of params out of allowed boundaries\n";
    exit(INCORRECT_PARAM);


}

void FreeMenuItemsMemory(int size){
//    for(int i=0;i<size;i++)
//        delete menuItems[i];
//    delete [] menuItems;
//
//    cout << "Memory deallocated\n";
}

void createMenuItems(int itemsAmount){

    for (int j = 0; j < itemsAmount; j++){
        menuItems.push_back(MenuItem(j,ItemNameList[j],ItemPrices[j]));

    }

    allocateSharedMemory(getpid(),&itemsSegptr,&itemsShmid);

    itemsVectorToShmem(menuItems,itemsShmid);

    cout << "============= Menu ============="<<endl;
    cout << "ID" <<setw(7)<<"Name"<<setprecision(5)<<setw(8)<<fixed<< setprecision(2)<<"Price" <<setw(3)<<"Orders" <<endl;
    for(vector<MenuItem>::iterator it = menuItems.begin();it!=menuItems.end();++it)
        cout << it->getID()+1<<setw(15)<< it->getName()<<setprecision(5)<<setw(8)<<fixed<< setprecision(2)<<it->getPrice() <<setw(3)<<it->getTotalOrdered() <<endl;



}


string getTime(){

        gettimeofday(&currentTime,0);
        std::ostringstream os;
        float timestep = (currentTime.tv_sec - startTime.tv_sec) + (float)(currentTime.tv_usec - startTime.tv_usec)/1000000;
        os <<fixed<< setprecision(4) << timestep << "\t| ";
    return os.str();

}

void createEarningCounter(){
    allocateSharedMemory(getpid(),&earningsSegptr,&earningsShmid);
    strcpy(earningsSegptr,"0");

    allocateSharedMemory(getpid(),&orderedAmountSegptr,&orderedAmountShmid);
    strcpy(orderedAmountSegptr,"0");
}

void increaseEarnings(float earnedNow){
    float earnedBefore = atof(earningsSegptr);
    earnedBefore+=earnedNow;
    char array[15];
    sprintf(array, "%f", earnedBefore);
    strcpy(earningsSegptr,array);

}


void increaseOrdersAmount(int orderedNow){
    int orderedBefore = atoi(orderedAmountSegptr);
    orderedBefore+=orderedNow;
    char array[15];
    sprintf(array, "%d", orderedBefore);
    strcpy(orderedAmountSegptr,array);
}


void createOrdersBoards(int boardsAmount){


    allocateSharedMemory(getpid(),&ordersSegptr,&ordersShmid);
    vector<OrderBoard> orderBoard;
    for (int i =0; i< boardsAmount;i++)
        orderBoard.push_back(OrderBoard(i,-1,0,true));

    ordersVectorToShmem(orderBoard,ordersShmid);

    createEarningCounter();

}

void startClientsProcesses(int clientsSize, int waitersSize){
    pid_t pid=-1;
    int myID=-1;
    clientType  type=PARENT;
    for(int i=0;i<clientsSize;i++){   //waiters
        if((pid=fork())==0){
            myID = i;
             type = CUSTOMER;
            cout << getTime()<<"Customer " << myID <<": created PID " << getpid() << " PPID " << getppid() << endl;

            break;
        }
    }
    if(pid!=0)
        for(int i=0;i<waitersSize;i++){   //waiters
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
        exit(1);
    }

    if (type==WAITER)
        manageWaiters(getpid(),myID);
    else if (type == CUSTOMER)
        manageCustomers(getpid(),myID);
    else
    {
        int wpid,status=0;
        while ((wpid = wait(&status)) > 0);
        manageMainProcess(getpid());
    }

}

float getRandomSleepTime(int min,int max){
    std::random_device  gen;
    std::uniform_real_distribution<> uid(min, max);
    return uid(gen)*1000000; // for seconds

}

int getRandNum(int min,int max){
    std::random_device  gen;
    std::uniform_int_distribution<> uid(min, max);
    return uid(gen);
}

bool isTimedOut(){
    return (simulationDuration<(currentTime.tv_sec-startTime.tv_sec));
}

bool isPrevOrderDone(int custID){

    if(!isTimedOut()) {

        vector<OrderBoard> orders = ordersShmemToVector();
        for (vector<OrderBoard>::iterator it = orders.begin(); it != orders.end(); ++it)
            if (custID == it->getCustomerId()) {
                if (it->isDone() == true)
                    return true;
                 else
                    return false;

            }
        orders.clear();
    }

    return false;
}

void manageCustomers(pid_t pid,int id){
    float sleepTime;

    while (!isTimedOut()){

        sleepTime = getRandomSleepTime(3,6);
        usleep(sleepTime);

        if (!isPrevOrderDone(id)){
            continue;
        }
        else {
                vector<MenuItem> menu = itemsShmemToVector();
                MenuItem itemToOrder = randomChooseItem(menu);

                bool isOrdered = (getRandNum(0, 1) == 1) ? true : false;

                   // reads menu
                if (isOrdered) {
                    p(customersSem);
                    cout << getTime() << "Customer " << id << ": reads menu about " << itemToOrder.getName();
                    sleep(1);
                    int amount = getRandNum(1, 4);
                    cout << "( ordered, amount " << amount << " )\n";
                    v(customersSem);

                    // only one customer can add items to board, and must to block entrance to orders for other processes
                    p(customersSem);
                    p(ordersSem);
                    addOrderToBoard(id, itemToOrder.getID(), amount);
                    v(ordersSem);
                    v(customersSem);

                } else {

                    cout << getTime() << "Customer " << id << ": reads menu about " << itemToOrder.getName();

                    sleep(1);

                    cout << "( doesn't want to order )" << endl;


                }

        }
    }


    cout << getTime()<<"Customer ID "<<id<< " PID "<<getpid()<<" end work PPID "<<getppid()<<endl;

}


//bool isThereUndoneOrders(){
//    vector<OrderBoard> orders = ordersShmemToVector();
//    for(vector<OrderBoard>::iterator it = orders.begin();it!=orders.end();++it)
//        if(it->isDone()==false)
//            return true;
//
//    return false;
//}

void manageWaiters(pid_t pid,int waiterID){
    int sleepTime;
    while (!isTimedOut()){
        sleepTime = getRandomSleepTime(1,2);
        usleep(sleepTime);

        vector<OrderBoard> orders = ordersShmemToVector();

        for(vector<OrderBoard>::iterator it = orders.begin();it!=orders.end();++it){
            if(!it->isDone())
            {

                cout << getTime()<<"Waiter ID "<<waiterID<< ": performes the order of customer "<<it->getCustomerId()<<" ("<<it->getAmount()<<" "<<ItemNameList[it->getItemId()]<<")\n";

                vector<MenuItem> menu =  itemsShmemToVector();

                updateOrderedAmount(menu,it->getItemId(),it->getAmount());
                it->setDone(true);
                ordersVectorToShmem(orders,ordersShmid);

            }
        }

        orders.clear();

    }

    cout << getTime()<<"Waiter ID "<<waiterID<< " PID "<<getpid()<<" end work PPID "<<getppid()<<endl;

}


void manageMainProcess(pid_t pid){
    menuItems.clear();

    menuItems = itemsShmemToVector();
    cout << "============= Menu ============="<<endl;
    cout << "ID" <<setw(15)<< "Name"<<setprecision(5)<<setw(8)<<fixed<< setprecision(2)<<"Price" <<setw(3)<<"Orders" <<endl;
    for(vector<MenuItem>::iterator it = menuItems.begin();it!=menuItems.end();++it)
        cout << it->getID()+1<<setw(15)<< it->getName()<<setprecision(5)<<setw(8)<<fixed<< setprecision(2)<<it->getPrice() <<setw(3)<<it->getTotalOrdered() <<endl;

    cout << "Total earned : "<<atof(earningsSegptr)<<" $\n";
    cout << "Total orders : "<<orderedAmountSegptr<<endl;
    cout << getTime() << "Main ID "<<getpid()<< " end work"<<endl;
    cout << getTime()<<"End of simulation\n";

    endBanner();
    deleteSemsAndShmems();
}


void printMenu(char* segptr){
    // read from shared memory
    char* c=segptr;
    while (*c)
    {
        if(*c=='|')
        {
            cout <<" | ";
            c++;
        }

        if (*c !='\n')
            cout << *c;
        else
           cout <<endl;
        c++;
    }

//    cout << segptr << endl;
//    for(vector<MenuItem>::iterator it = menuItems.begin();it!=menuItems.end();++it)
//        cout << it->getID()+1<<setw(15)<< it->getName()<<setprecision(5)<<setw(8)<<fixed<< setprecision(2)<<it->getPrice() <<setw(3)<<it->getTotalOrdered() <<endl;
//

}

void allocateSharedMemory(pid_t pid,char** segptr, int *shmid){
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
    if((*segptr = (char*)shmat(*shmid, 0, 0)) == NULL)
    {
        perror("shmat");
        exit(1);
    }
}

void deleteSemsAndShmems(){
shmctl(itemsShmid, IPC_RMID, 0);    // items shmem
shmctl(ordersShmid, IPC_RMID, 0);    // items shmem
shmctl(orderedAmountShmid, IPC_RMID, 0);    // items shmem
shmctl(earningsShmid, IPC_RMID, 0);    // items shmem

semctl(itemsSem, 0, IPC_RMID, 0);   // items sem
semctl(customersSem, 0, IPC_RMID, 0);   // items sem
semctl(ordersSem, 0, IPC_RMID, 0);   // orders sem
semctl(waitersSem, 0, IPC_RMID, 0);   // waiters sem
semctl(coutSem, 0, IPC_RMID, 0);   // waiters sem
semctl(reader, 0, IPC_RMID, 0);   // waiters sem
semctl(writer, 0, IPC_RMID, 0);   // waiters sem
}




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

    while(std::getline(ss,token,'|')){
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


// for friendly use, get data from shared memory and convert into vector array
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

void itemsVectorToShmem(vector<MenuItem> items,int itemsshmID){


    strcpy(itemsSegptr,"");
    for(vector<MenuItem>::iterator it = items.begin();it!=items.end();++it)
    {
        //write to shared memory
        strcat(itemsSegptr, it->to_string().c_str());
        strcat(itemsSegptr,"\n");
    }


}

void ordersVectorToShmem(vector<OrderBoard> items,int ordersShmemID){


    strcpy(ordersSegptr,"");
    for(vector<OrderBoard>::iterator it = items.begin();it!=items.end();++it)
    {
        //write to shared memory
        strcat(ordersSegptr, it->to_string().c_str());
        strcat(ordersSegptr,"\n");
    }


}

void updateOrderedAmount(vector<MenuItem> items, int itemID,int amount){

    for (vector<MenuItem>::iterator it = items.begin();it!=items.end();++it)
        if (it->getID()==itemID)
        {
            it->setTotalOrdered(it->getTotalOrdered()+amount);
            increaseEarnings(it->getPrice()*amount);
            increaseOrdersAmount(amount);
        }

    itemsVectorToShmem(items,itemsShmid);

}

void initAllSemaphores(){
    srand(time(NULL));
    key_t semkey   = ftok(".", getRandNum(0,100000));
    if((itemsSem=initsem(semkey,1))<0)
        exit(1);

    semkey   = ftok(".", getRandNum(0,100000));
    if((customersSem=initsem(semkey,1))<0)
        exit(1);

    semkey   = ftok(".", getRandNum(0,100000));
    if((ordersSem=initsem(semkey,1))<0)
        exit(1);

    semkey   = ftok(".", getRandNum(0,100000));
    if((waitersSem=initsem(semkey,1))<0)
        exit(1);
    semkey   = ftok(".", getRandNum(0,100000));
    if((coutSem=initsem(semkey,1))<0)
        exit(1);


    semkey   = ftok(".", getRandNum(0,100000));
    if((reader=initsem(semkey,1))<0)
        exit(1);

    semkey   = ftok(".", getRandNum(0,100000));
    if((writer=initsem(semkey,1))<0)
        exit(1);

}


void addOrderToBoard(int customerID, int itemID,int amount){

    vector<OrderBoard> ordersList = ordersShmemToVector();

    for(vector<OrderBoard>::iterator it = ordersList.begin();it!=ordersList.end();++it)
        if(it->getCustomerId() == customerID)
        {
            it->setItemId(itemID);
            it->setAmount(amount);
            it->setDone(false);
        }

    ordersVectorToShmem(ordersList,ordersShmid);





}



#endif //HW3_EX3_H
