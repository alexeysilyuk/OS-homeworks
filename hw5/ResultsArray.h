//
// Created by alex on 5/4/18.
//

#ifndef HW5_RESULTSARRAY_H
#define HW5_RESULTSARRAY_H

#include <string>
#include <fstream>
#include <string>
#include <iostream>
#include "RWLock.h"

struct resultNode{
    string hostname;
    string ip;
    resultNode* next;

};

class ResultsArray{
private:
    resultNode *head, *tail;
    pthread_mutex_t rlock,wrlock;
    RWLock rwLock;
    string outputFileName;
    FILE *file= NULL;
public:
    ResultsArray(){
        head=NULL;
        tail=NULL;
        pthread_mutex_init(&rlock,NULL);
        pthread_mutex_init(&wrlock,NULL);
    }
    ~ResultsArray(){
        pthread_mutex_destroy(&rlock);
        pthread_mutex_destroy(&wrlock);
    }

    void setOutputFileName(string filename){
        outputFileName=filename;

        pthread_mutex_lock(&wrlock);
        file = fopen(filename.c_str(),"w");
        if (file== NULL)
        {
            cerr << "ERROR OPENING FILE \'"<<filename<<"\'\n";;
            exit(OUTPUT_FILE_NOT_EXISTS);
        }
        fclose(file);
        pthread_mutex_unlock(&wrlock);
    }


    void writeToFile(string line){
        pthread_mutex_lock(&wrlock);
        file = fopen(outputFileName.c_str(),"a");// use "a" for append, "w" to overwrite, previous content will be deleted
        if (file== NULL)
        {
            cerr << "ERROR OPENING AND WRITING TO FILE\n";
            exit(OUTPUT_FILE_NOT_EXISTS);
        }

        fprintf(file,"%s",line.c_str());// newline

        fclose (file); // must close after opening
        pthread_mutex_unlock(&wrlock);
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
       string line =host+","+ip+'\r';
        cout << line<<endl;
        writeToFile(line);
        rwLock.WriteUnlock();
    }


    bool contains(string hostname)
    {
        rwLock.ReadLock();
        resultNode *temp;
        temp=head;

        while(temp!=NULL)
        {
            if (hostname.compare(temp->hostname) == 0) {
//                cout << "hostname " << hostname << " been processed before and has ip: " << temp->ip << endl;
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
