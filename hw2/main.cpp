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
string parseVar(string it);

int lastExitStatus=0;

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
            exit(lastExitStatus);
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
            if(it!=toks.end())
            {
                parseVar(*it);
                return;
            }
            else
            {
                perror("@Shell: cd : command must receive argument");
                lastExitStatus=1;
            }

        }
        else
            cout << "OTHER COMMAND"<<endl;
    }
}
bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

string parseVar(string it){
    string parsedPath="";
    string varName;

    for(int i=0,j=0;i<it.length();i++)
    {
        if(it[i]=='$'){
            i++;
            if(it[i]=='?')
                parsedPath+=to_string(lastExitStatus);
            else
            {
                varName="";
                while(it[i] !='/' && i<it.length()) {
                    if ((it[i] >= 'a' && it[i] <= 'z') || (it[i] >= 'A' && it[i] <= 'Z') || (it[i]=='_') || (it[i]>='0' && it[i]<='9'))
                        varName += it[i++];
                    else
                        cout <<"Incorrect char in variable name\n";
                }

                try{
                    char* varPath = getenv (varName.c_str());
                    if(varPath)
                    {
                        cout << "Var path: "<<varPath<<endl;
                        parsedPath+=varPath;
                    }
                }
                catch(exception ex)
                {

                }
                parsedPath+=it[i];
            }
        }
        else
            parsedPath+=it[i];

    }
cout << "PATH: " <<parsedPath<<endl;
    //replace(it, "$?", to_string(lastExitStatus));

//
//    char* pPath = getenv ("VAR");
//    if (pPath!=NULL)
//        replace(it, "$VAR", pPath);
//    else
//        cout << "$VAR not exists in environment\n";
//    cout << "after :"<<it<<endl;


//
//    str.replace("$?","TEST");
//    if(str.replace("$?",lastExitStatus+'')){
//        cout << lastExitStatus<< endl;
//    }
//
//
//
//    cout << it->find('$')<<endl;
//    if(it->compare("~")==0)
//    {
//        homedir = getenv("HOME");
//        if(chdir(homedir.c_str())==0)
//            lastExitStatus=0;
//        else
//        {
//            perror("@Shell: cd");
//            lastExitStatus=1;
//        }
//    }
//    else if (chdir(it->c_str()) == 0)
//    {
//        char cwd[512];
//        currentDir = getcwd(cwd, sizeof(cwd));
//        lastExitStatus=0;
//    }
//    else
//    {
//        perror("@Shell: cd");
//        lastExitStatus=1;
//    }
}