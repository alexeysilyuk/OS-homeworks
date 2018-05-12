//
// Created by alex on 5/4/18.
//

#ifndef HW5_RESULTSARRAY_H
#define HW5_RESULTSARRAY_H

//  defining colors for ptinting
#include "colormod.h"
Color::Modifier redC(Color::BG_RED);
Color::Modifier redF(Color::FG_RED);
Color::Modifier defF(Color::FG_DEFAULT);
Color::Modifier defB(Color::BG_DEFAULT);
Color::Modifier greF(Color::FG_GREEN);
Color::Modifier greB(Color::BG_GREEN);
Color::Modifier blueB(Color::BG_BLUE);
Color::Modifier blueF(Color::FG_BLUE);


#include <string>
#include <fstream>
#include <string>
#include <iostream>

struct resultNode{
    string hostname;
    string ip;
    resultNode* next;

};
int i=0;    // var for printing results with number
class ResultsArray{
private:
    resultNode *head, *tail;
    pthread_mutex_t dumplock,filelock,rwlock,contLock;
    string outputFileName;      // file results will be printed
    FILE *file= NULL;
public:
    // initialize and destroy mutexes
    ResultsArray(){
        head=NULL;
        tail=NULL;
        pthread_mutex_init(&rwlock,NULL);
        pthread_mutex_init(&contLock,NULL);
        pthread_mutex_init(&filelock,NULL);
        pthread_mutex_init(&dumplock,NULL);
    }
    ~ResultsArray(){
        pthread_mutex_destroy(&dumplock);
        pthread_mutex_destroy(&filelock);
        pthread_mutex_destroy(&rwlock);
        pthread_mutex_destroy(&contLock);
    }

    // set file name to print results to
    void setOutputFileName(string filename){
        outputFileName=filename;

        // lock resource, and try to open file for reading,
        // if file opened successfully means file exists, otherwise throw error and exit
        pthread_mutex_lock(&filelock);
        file = fopen(filename.c_str(),"w");// w option will remove prevoius data from existing file
        if (file== NULL)
        {
            cerr << "! ERROR OPENING FILE \'"<<filename<<"\'\n";;
            exit(OUTPUT_FILE_NOT_EXISTS);
        }
        fclose(file);
        pthread_mutex_unlock(&filelock);
    }

    // function to write line to shared file
    void writeToFile(string line){

        // try to open file for appending
        pthread_mutex_lock(&filelock);
        file = fopen(outputFileName.c_str(),"a");// use "a" for append
        if (file== NULL)
        {
            cerr << "! ERROR OPENING AND WRITING TO FILE\n";
            exit(OUTPUT_FILE_NOT_EXISTS);
        }

        fprintf(file,"%s",line.c_str());// newline

        fclose (file); // must close after opening
        pthread_mutex_unlock(&filelock);
    }

    // add new resolved request to tail of linked list with results,
    // using mutex for prevent few pthreads to add simultaniously
    void addTail(string host, string ip)
    {
        pthread_mutex_lock(&rwlock);

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
        cout << "\t"<<++i<<") "<<line<<endl;

        pthread_mutex_unlock(&rwlock);
        // print added result to user

    }

    // checking if given hostname already been processe before and added to result list
    bool contains(string hostname)
    {
        pthread_mutex_lock(&contLock);
        resultNode *temp;
        temp=head;

        while(temp!=NULL)
        {
            // if found this hostname in results, returns true otherwise after ending return false
            if (hostname.compare(temp->hostname) == 0) {
                pthread_mutex_unlock(&contLock);
                return true;
            }
            temp=temp->next;
        }
        pthread_mutex_unlock(&contLock);
        return false;
    }

    // check fi there are results in array to take
    bool isEmpty(){
        return (head==NULL)? true:false;
    }

    // remove first element from linked list and returns it to dumper thread for printing to file
    string popFirst(){

        pthread_mutex_lock(&contLock);
        string res="NULL";
        resultNode * temp = head;
        if(head!=NULL){
            res =temp->hostname+","+temp->ip+'\r';
            head=temp->next;
            delete temp;
        }
        pthread_mutex_unlock(&contLock);

        return res;

    }

////     print whole array
//    void print(){
//        resultNode *temp;
//        temp=head;
//
//        while(temp!=NULL)
//        {
//            cout << temp->ip<<endl;
//            temp=temp->next;
//        }
//    }
};


#endif //HW5_RESULTSARRAY_H
