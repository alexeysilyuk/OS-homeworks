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

using namespace std;


string ItemNameList[] =  {"Pizza","Salad","Hamburger","Spaghetti","Pie","Milkshake","Pelmeni","Salo","Borsh","Olivie"};
float ItemPrices[] = {10.00,7.50,12.00,9.00,9.50,6.00,8.00,3.99,8.10,5.30};
enum clientType {CUSTOMER,WAITER,PARENT};

enum ERROR_CODES {INCORRECT_PARAM=128};
#define SEGSIZE 100*10 // 100 bytes for each item * maximum 10 items in menu


char  *itemsSegptr, *ordersSegptr;
int ordersShmid,itemsShmid;


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
MenuItem randomChooseItem(char* itemsSegptr);
vector<MenuItem> shmemToVector();
char* ordersVectorToShmem(vector<MenuItem> items);
void updateOrderedAmount(int amount, int itemID);

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
//
    allocateSharedMemory(getpid(),&itemsSegptr,&itemsShmid);

    for(vector<MenuItem>::iterator it = menuItems.begin();it!=menuItems.end();++it)
    {
        //write to shared memory
        strcat(itemsSegptr, it->to_string().c_str());
        strcat(itemsSegptr,"\n");

    }

    printMenu(itemsSegptr);

    //remove a shared memory section
    //IPC_RMID: Marks a segment for removal.
    shmctl(itemsShmid, IPC_RMID, 0);
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

int getRandNum(pid_t pid,int minTime,int maxTime){
    std::mt19937 gen(time(0)+pid);
    std::uniform_int_distribution<> uid(minTime, maxTime);
    return uid(gen);
}

bool isTimedOut(){
    return (simulationDuration<(currentTime.tv_sec-startTime.tv_sec));
}

void manageCustomers(pid_t pid,int id){
    int sleepTime;
    while (!isTimedOut()){
        sleepTime = getRandNum(getpid(),3,6);

        sleep(sleepTime);

        bool isOrdered = (getRandNum(getpid(),0,1)==1)?true:false;
        MenuItem itemToOrder = randomChooseItem(itemsSegptr);
        cout << getTime()<< "Customer "<<id << ": reads menu about "<<itemToOrder.getName();

        sleep(1);

        if(isOrdered){
            int amount = getRandNum(getpid(),1,4);
            cout << "( ordered, amount "<<amount<<" )\n";
            updateOrderedAmount(amount,itemToOrder.getID());
        }
        else
            cout << "( doesn't want to order )"<<endl;



    }
}

void manageWaiters(pid_t pid){
    int sleepTime;
    while (!isTimedOut()){
        sleepTime = getRandNum(getpid(),3,6);
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
    int randNum = getRandNum(pid,0,100000);
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

void manageMainProcess(pid_t pid){

}

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
    MenuItem newItem = MenuItem(id,name,price,orders);

    return newItem;



}

MenuItem randomChooseItem(char* itemsSegptr)
{
    int randomItemIndex = getRandNum(getpid(),0,menuItemsCount-1);
        std::stringstream ss(itemsSegptr);
        std::string token;
        int line=0;
        if (itemsSegptr != NULL)
        {
            while(std::getline(ss,token,'\n')){

                if(line==randomItemIndex){
                    return getTokens(token);
                }

                line++;
            }
        }


}

// for friendly use, get data from shared memory and convert into vector array
vector<MenuItem> shmemToVector(){
    vector<MenuItem> itemsList;
    std::stringstream ss(itemsSegptr);
    std::string token;
    int line=0;
    if (itemsSegptr != NULL)
    {
        while(std::getline(ss,token,'\n')){
            itemsList.push_back(getTokens(token));
        }
    }
}
char* ordersVectorToShmem(vector<MenuItem> items){

}

void updateOrderedAmount(int amount, int itemID){
    vector<MenuItem> items = shmemToVector();
//    MenuItem itemToUpdate = items[itemID];
    cout << "vectorSize() = "<<items.size()<<endl;
//
//    for(int i=0;i<items.size();i++)
//        cout << items.at(i).getID()<<endl;

}
#endif //HW3_EX3_H
