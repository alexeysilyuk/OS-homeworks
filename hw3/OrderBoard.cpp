//
// Created by alex on 4/10/18.
//

#include "OrderBoard.h"

int OrderBoard::getCustomerId() const {
    return CustomerId;
}

void OrderBoard::setCustomerId(int CustomerId) {
    OrderBoard::CustomerId = CustomerId;
}

int OrderBoard::getAmount() const {
    return Amount;
}

void OrderBoard::setAmount(int Amount) {
    OrderBoard::Amount = Amount;
}

int OrderBoard::getItemId() const {
    return ItemId;
}

void OrderBoard::setItemId(int ItemId) {
    OrderBoard::ItemId = ItemId;
}

bool OrderBoard::isDone() const {
    return Done;
}

void OrderBoard::setDone(bool Done) {
    OrderBoard::Done = Done;
}
