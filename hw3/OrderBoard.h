//
// Created by alex on 4/10/18.
//

#ifndef HW3_ORDERBOARD_H
#define HW3_ORDERBOARD_H

#include <iostream>
#include "MenuItem.h"
class OrderBoard {
private:
    bool Done=true;
    int CustomerId=0,Amount=0, ItemId=0;

public:
    int getCustomerId() const;
    void setCustomerId(int CustomerId);
    int getAmount() const;
    void setAmount(int Amount);
    int getItemId() const;
    void setItemId(int ItemId);
    bool isDone() const;
    void setDone(bool Done);


    OrderBoard(int customerID):CustomerId(customerID){};

};


#endif //HW3_ORDERBOARD_H