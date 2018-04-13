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


char  *itemsSegptr, *ordersSegptr;
int ordersShmid,itemsShmid;
int itemsSem,ordersSem,waitersSem,customersSem;


vector<MenuItem> menuItems;
vector<OrderBoard> boards;
//MenuItem** menuItems;
//OrderBoard** boards;
int simulationDuration,menuItemsCount;

struct timeval currentTime, startTime;


/// FUNCS Predeclarations
void manageCustomers(pid_t pid,int id);
void manageWaiters(pid_t pid);
bool isTimedOut();
void allocateSharedMemory(pid_t pid,char** segptr, int *shmid);
void printMenu(char* segptr);
void manageMainProcess(pid_t pid);
MenuItem randomChooseItem(vector<MenuItem> menu);
vector<MenuItem> shmemToVector();
char* ordersVectorToShmem(vector<MenuItem> items);
void updateOrderedAmount(vector<MenuItem> items, int itemID,int amount);
void itemsVectorToShmem(vector<MenuItem> items,int itemsShmid);
void initAllSemaphores();
////////////////////////////////

bool analyzeParams(int argc, char * argv[]){
    for (int i=1;i<argc;i++)
    {
        try{
            stoi(argv[i]);
        }
        catch(exception ex)
        {
            cout << "At least one argument has incorrect type\n"<<endl;
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
    cout << "One of params out of allowed boundaries\n";
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

    printMenu(itemsSegptr);

    //remove a shared memory section
    //IPC_RMID: Marks a segment for removal.
//    shmctl(itemsShmid, IPC_RMID, 0);
}


string getTime(){

        gettimeofday(&currentTime,0);
        std::ostringstream os;
        float timestep = (currentTime.tv_sec - startTime.tv_sec) + (float)(currentTime.tv_usec - startTime.tv_usec)/1000000;
        os <<fixed<< setprecision(4) << timestep << " | ";
    return os.str();

}


void createOrdersBoards(int boardsAmount){

    for (int i=0;i<boardsAmount;i++)
        boards.push_back(OrderBoard(i));

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
        cout << "fork() error\n";
        exit(1);
    }

    if (type==WAITER)
        manageWaiters(getpid());
    else if (type == CUSTOMER)
        manageCustomers(getpid(),myID);
    else
    {
        int wpid,status=0;
        while ((wpid = wait(&status)) > 0);
        manageMainProcess(getpid());
    }

}

int getRandNum(int min,int max){
    std::random_device  gen;
    std::uniform_int_distribution<> uid(min, max);
    return uid(gen);




}

bool isTimedOut(){
    return (simulationDuration<(currentTime.tv_sec-startTime.tv_sec));
}

void manageCustomers(pid_t pid,int id){
    int sleepTime;

    while (!isTimedOut()){
        sleepTime = getRandNum(3,6);
        sleep(sleepTime);
        p(customersSem);
        vector<MenuItem> menu =  shmemToVector();
        MenuItem itemToOrder = randomChooseItem(menu);
        cout << getTime()<< "Customer "<<id << ": reads menu about "<<itemToOrder.getName();
        bool isOrdered = (getRandNum(0,1)==1)?true:false;
        sleep(1);

        if(isOrdered){
            int amount = getRandNum(1,4);
            cout << "( ordered, amount "<<amount<<" )\n";
            updateOrderedAmount(menu,itemToOrder.getID(),amount);
        }
        else
            cout << "( doesn't want to order )"<<endl;
        v(customersSem);
    }

}

void manageWaiters(pid_t pid){
    int sleepTime;
    while (!isTimedOut()){
        sleepTime = getRandNum(3,6);
        sleep(sleepTime);
        cout << getTime()<<"waiter "<<pid<< " after sleep "<< sleepTime<<endl;
    }
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
semctl(itemsSem, 0, IPC_RMID, 0);   // items sem
semctl(customersSem, 0, IPC_RMID, 0);   // items sem
}

void manageMainProcess(pid_t pid){
    vector<MenuItem> list = shmemToVector();
    for(vector<MenuItem>::iterator it = list.begin();it!=list.end();++it)
        cout<<it->to_string()<<endl;

    deleteSemsAndShmems();
}


MenuItem randomChooseItem(vector<MenuItem> menu)
{
    int randomItemIndex = getRandNum(0,menu.size()-1);
    return menu.at(randomItemIndex);

}

// receive line from shmem and sepearate it by '|' delimiter, and create MenuItem object
MenuItem getTokens(string tokenLine){
    list<string> tokens;
    std::stringstream ss(tokenLine);
    std::string token;

    while(std::getline(ss,token,'|')){
        tokens.push_back(token);
    }

    int id =  atoi(tokens.front().c_str());
    tokens.pop_front();
    string name = tokens.front();
    tokens.pop_front();
    float price = stof(tokens.front());
    tokens.pop_front();
    int orders =  atoi(tokens.front().c_str());
    tokens.pop_front();



    return MenuItem(id,name,price,orders);
}


// for friendly use, get data from shared memory and convert into vector array
vector<MenuItem> shmemToVector(){
    vector<MenuItem> itemsList;

    p(itemsSem);
    std::stringstream ss(itemsSegptr);
    std::string token;
    if (itemsSegptr != NULL)
    {
        while(std::getline(ss,token,'\n')){
            MenuItem item = getTokens(token);
            itemsList.push_back(item);

        }
    }
    v(itemsSem);
    return itemsList;
}
void itemsVectorToShmem(vector<MenuItem> items,int itemsshmID){


    p(itemsSem);

    strcpy(itemsSegptr,"");
    for(vector<MenuItem>::iterator it = items.begin();it!=items.end();++it)
    {
        //write to shared memory
        strcat(itemsSegptr, it->to_string().c_str());
        strcat(itemsSegptr,"\n");

    }
    v(itemsSem);

}

void updateOrderedAmount(vector<MenuItem> items, int itemID,int amount){

    for (vector<MenuItem>::iterator it = items.begin();it!=items.end();++it)
        if (it->getID()==itemID)
            it->setTotalOrdered(it->getTotalOrdered()+amount);

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

}

#endif //HW3_EX3_H
