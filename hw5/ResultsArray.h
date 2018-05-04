//
// Created by alex on 5/4/18.
//

#ifndef HW5_RESULTSARRAY_H
#define HW5_RESULTSARRAY_H

#include <string>
#include "RWLock.h"

struct resultNode{
    string hostname;
    string ip;
    resultNode* next;

};

class ResultsArray{
private:
    resultNode *head, *tail;
    pthread_rwlock_t rlock,wrlock;
    RWLock rwLock;

public:
    ResultsArray(){
        head=NULL;
        tail=NULL;
    }

    void addTail(string host, string ip)
    {
        rwLock.WriteLock();
        resultNode *newResult=new resultNode;
        newResult->hostname = host;
        newResult->ip=ip;
        newResult->next=NULL;

        if(head==NULL)
        {
            head=newResult;
            tail=newResult;
            newResult=NULL;
        }
        else
        {
            tail->next=newResult;
            tail=newResult;
        }
        rwLock.WriteUnlock();
    }

    void printList()
    {
        resultNode *temp=new resultNode;
        temp=head;
        while(temp!=NULL)
        {
            cout<<temp->hostname<<":"<<temp->ip<<endl;
            temp=temp->next;
        }
    }

    bool contains(string hostname)
    {
        rwLock.ReadLock();
        resultNode *temp;
        temp=head;
        pthread_rwlock_rdlock(&rlock);
        while(temp!=NULL)
        {
            if (hostname.compare(temp->hostname) == 0) {
                cout << "hostname :" << hostname << " been processed before and has ip: " << temp->ip << endl;
                rwLock.ReadUnlock();
                return true;
            }


            temp=temp->next;
        }
        rwLock.ReadUnlock();
        return false;
    }
};


#endif //HW5_RESULTSARRAY_H
