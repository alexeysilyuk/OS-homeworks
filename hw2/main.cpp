#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <iterator>


using namespace std;

vector<string> read();
string eval(vector<string> tokens);
void cleanBuffer();
string getDir();
vector<string> parseLine(string line);

int main() {
    cout << "Welcome to \"@Shell\" " <<endl;
    while(true){
        cout << "@Shell: "<<getDir()<< "> ";

        vector<string> tokens = read();
        eval(tokens);

    }
    std::cout << "Hello, World!" << std::endl;
    return 0;
}

vector<string> read(){
    cleanBuffer();
    string line;


    do {
        getline(cin, line);
        if (cin.eof() || line.compare("exit") == 0)
        {
            cout << "Bye bye!" << endl;
            exit(0);
        }
        vector<string> tokens = parseLine(line);
        if(tokens.size()>0)
            return tokens;

    } while(true);



}

void cleanBuffer(){
    cin.clear();							// clear error flags
    cin.ignore(cin.rdbuf()->in_avail());	// ask buffer for length of chars in it and clear all of them

}
string getDir(){

    char buffer[1000];
    string homedir = getenv("HOME");
    string currentDir =getcwd(buffer,sizeof(buffer));

    if(currentDir.length()>homedir.length())
        currentDir="~"+currentDir.substr(homedir.length());

    else
        currentDir = getcwd(buffer,sizeof(buffer)); // return the current directory

    return  currentDir;
}

vector<string> parseLine(string line){
    string buf; // Have a buffer string
    stringstream ss(line); // Insert the string into a stream

    vector<string> tokens; // Create vector to hold our words

    while (ss >> buf)
        tokens.push_back(buf);

    return tokens;
}

string eval(vector<string> tokens){

    cout << "Tokens :"<<endl;
    for (vector<string>::const_iterator iter = tokens.begin();
         iter != tokens.end(); ++iter)
        cout << *iter << endl;
}