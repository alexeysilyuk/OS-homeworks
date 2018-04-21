//
// Created by alex on 4/10/18.
//

#ifndef HW3_MENUITEM_H
#define HW3_MENUITEM_H

#include <iostream>
#include <sstream>

class MenuItem {
private:
    int  ID;
    float Price;
    int TotalOrdered=0;
public:


private:
    std::string Name;

public:
    void setID(int _id)    {ID=_id;}
    int getID()    { return ID;}

    bool setPrice(float price)
    {
        if(price<=100)
        {
            Price=price;
            return true;
        }
        return false;
    }


    float getPrice()
    {
        return Price;
    }


    bool setName(std::string name)
    {
        if(name.size()<=15)
        {
            Name=name;
            return true;
        }
        return false;
    }


    std::string getName()
    {
        return Name;
    }

    int getTotalOrdered(){return TotalOrdered;}

    void setTotalOrdered(int TotalOrdered) {
        MenuItem::TotalOrdered = TotalOrdered;
    }

    MenuItem(int id, std::string name, float price){
        setID(id);
        setName(name);
        setPrice(price);
    }

    MenuItem(int id, std::string name, float price,int orders){
        setID(id);
        setName(name);
        setPrice(price);
        setTotalOrdered(orders);
    }

    // convert element to string
    std::string to_string(){
        std::ostringstream oss;
        oss<< ID<<"|"<<Name<<"|"<<Price<<"|"<<TotalOrdered;
        return oss.str();
    }
};


#endif //HW3_MENUITEM_H
