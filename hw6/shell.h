//
// Created by alex on 4/3/18.
//

#ifndef HW2_MAIN_H
#define HW2_MAIN_H

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <list>
#include <sys/wait.h>
#include <cstring>
using namespace std;


void read();
void cleanBuffer();
string getDir();
list <string> tokenize(string line);
char evaluate();
string parseVar(string it);
bool isVariable(string str);
bool isAlphaNumeric(char ch);
bool isCapitalLetter(char ch);
void openFile(string fileName,int redirectionNumber,int *filedesc);


#endif //HW2_MAIN_H
