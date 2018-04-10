//
// Created by alex on 4/10/18.
//

#ifndef HW3_EX3_H
#define HW3_EX3_H

#include <iostream>
#include <iomanip>
#include <sys/time.h>
#include <vector>
#include <numeric>
#include <algorithm>


using namespace std;


string ItemNameList[] =  {"Pizza","Salad","Hamburger","Spaghetti","Pie","Milkshake","Pelmeni","Salo","Borsh","Olivie"};
float ItemPrices[] = {10.00,7.50,12.00,9.00,9.50,6.00,8.00,3.99,8.10,5.30};

enum ERROR_CODES {INCORRECT_PARAM=128};

MenuItem** menuItems;

struct timeval currentTime, startTime;

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
    for(int i=0;i<size;i++)
        delete menuItems[i];
    delete [] menuItems;

    cout << "Memory deallocated\n";
}

void createMenuItems(int itemsAmount){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9);


    menuItems = new MenuItem*[itemsAmount];
    int index;
    for (int j = 0; j < itemsAmount; j++){
        index = dis(gen);
        cout << "random num "<<index<<endl;
        menuItems[j] = new MenuItem(j,ItemNameList[index],ItemPrices[index]);
        cout << menuItems[j]->getID()+1<<setw(15)<< menuItems[j]->getName()<<setprecision(4)<<setw(5)<<fixed<< setprecision(2)<<menuItems[j]->getPrice() <<setw(3)<<menuItems[j]->getTotalOrdered() <<endl;
    }
}


string getTime(){

        gettimeofday(&currentTime,0);
        std::ostringstream os;
        float timestep = (currentTime.tv_sec - startTime.tv_sec) + (float)(currentTime.tv_usec - startTime.tv_usec)/1000000;
        os <<fixed<< setprecision(4) << timestep << " | ";
    return os.str();

}

#endif //HW3_EX3_H
