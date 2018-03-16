#include <iostream>

#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <list>




using namespace std;

void read();
void ev(list<string> toks);
void cleanBuffer();
string getDir();
list<string> tokenize(string line);
void clearList();
void parseCommand(list<string> toks);

list< std::string > tokens;
string homedir;
string currentDir;

int main() {
    cout << "Welcome to \"@Shell\" " <<endl;
    cout << "@Shell: "<<getDir()<< "> ";
    while(true){

        tokens.clear();
        read();

        parseCommand(tokens);
        cout << "@Shell: "<<getDir()<< "> ";


    }
}

void read(){
    string line;

        cleanBuffer();
        getline(cin, line);

        if (cin.eof() || line.compare("exit") == 0)
        {
            cout << "Bye bye!" << endl;
            exit(0);
        }

    tokenize(line);



}


string getDir(){

    char buffer[1000];

        homedir = getenv("HOME");
        currentDir = getcwd(buffer, sizeof(buffer));

        if (currentDir.length() > homedir.length())
            currentDir = "~" + currentDir.substr(homedir.length());

        else if(currentDir.length() == homedir.length())
            currentDir = "~/";
        else
            currentDir = getcwd(buffer, sizeof(buffer)); // return the current directory

        return currentDir;

}

list<string> tokenize(string line){
    string buf; // Have a buffer string
    stringstream ss(line); // Insert the string into a stream

        while (ss >> buf)
            tokens.push_back(buf);

    return tokens;

}




void cleanBuffer(){
    cin.clear();							// clear error flags
    cin.ignore(cin.rdbuf()->in_avail());	// ask buffer for length of chars in it and clear all of them

}



void parseCommand(list<string> toks){
    std::list<string>::iterator it;

    for (it=toks.begin(); it!=toks.end(); ++it)
    {

        if(it->compare("cd")==0)
        {
            ++it;
            if(&it!=NULL)
            {
                if (chdir(it->c_str()) == 0)
                {
                    char cwd[512];
                    currentDir = getcwd(cwd, sizeof(cwd));
                }
                else
                    perror("@Shell: cd");

            }
            else
                perror("cd command must receive argument");
        }
        else if(it->compare("$?")==0){
            cout << "LAST STATUS" <<endl;
        }
        else
            cout << "OTHER COMMAND"<<endl;
    }
}
