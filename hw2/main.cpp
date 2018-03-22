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
bool isVariable(string str);
bool isAlphaNumeric(char ch);
bool isCapitalLetter(char ch);

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
        if(it[i]=='$' && isCapitalLetter(it[i+1]))
        {
            i++;
                varName="";
                while(it[i] !='/' && i<it.length() && (isAlphaNumeric(it[i]) || it[i]=='_')) {
                        varName += it[i++];
                }
                cout << isVariable(varName)<<endl;
                try{
                    char* varPath = getenv (varName.c_str());
                    if(varPath)
                    {
                        cout << "Var path: "<<varPath<<endl;
                        parsedPath+=varPath;
                    }
                }
                catch(exception ex){}

            i--;
        }
        else if(it[i]=='$'&& it[i+1]=='?'){
            i++;
            parsedPath+=to_string(lastExitStatus);
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

bool isVariable(string str){
    bool status=false;
    if(isCapitalLetter(str[0]))
    {
        for(int i=1;i<str.length();i++)
            if(isAlphaNumeric(str[i])||str[i]=='_')
                status=true;
            else
                return false;
    }
    return status;
}

bool isAlphaNumeric(char ch){
    return ((ch>='A' && ch<='Z')||(ch>='a' && ch<='z')||(ch>='0' && ch<='9')) ?true:false;
}


bool isCapitalLetter(char ch){
    return (ch>='A' && ch<='Z') ? true : false;
}