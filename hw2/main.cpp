#include <iostream>

#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <list>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>


using namespace std;

void read();
void cleanBuffer();
string getDir();
list<string> tokenize(string line);
void clearList();
void evaluate();
string parseVar(string it);
bool isVariable(string str);
bool isAlphaNumeric(char ch);
bool isCapitalLetter(char ch);
void ChildProc(void); /* child process prototype */
void ParentProc(void); /* parent process prototype */
void runProccess(char **command);
void runBackProcess(char ** command);

#define MAX_ARGS 10
#define ARGSSIZE 1024
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

        evaluate();
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



void evaluate(){

    string path="";
    for (list<string>::const_iterator it = tokens.begin(), end = tokens.end(); it != end; ++it)
        {
        if(it->compare("cd")==0)
        {
            ++it;
            if(it!=tokens.end())
            {
                path=parseVar(*it);
                if(path.compare("~")==0)
                {
                    homedir = getenv("HOME");
                    if(chdir(homedir.c_str())==0)
                        lastExitStatus=0;
                    else
                    {
                        perror("@Shell: cd");
                        lastExitStatus=1;
                    }
                }
                else if (chdir(path.c_str()) == 0)
                {
                    char cwd[1024];
                    currentDir = getcwd(cwd, sizeof(cwd));
                    lastExitStatus=0;
                }

                else
                {
                    perror("@Shell: cd");
                    lastExitStatus=1;
                }

                return;
            }
            else
            {
                perror("@Shell: cd : command must receive argument");
                lastExitStatus=1;
            }

        }

            else if (it->compare("$?")==0)
        {
            cout << lastExitStatus<<endl;
            return;
        }
        else
        {
            int status;
            const char** array = new const char*[tokens.size()];
            char *args[ARGSSIZE];
            unsigned index = 0;
            bool deamonProc=false;

            for (list<string>::const_iterator iter = it; iter != tokens.end(); ++iter) {
                if(*iter == "&" && index+1==tokens.size())
                {
                    deamonProc=true;
                }
                else
                {
                    array[index] = iter->c_str();
                    args[index] = new char[ARGSSIZE];
                    strcpy(args[index],parseVar(array[index]).c_str());
                }

                index++;
            }
            pid_t zombiePid,pid;
            int WEXITSTATUS_status;

            switch(pid=fork()) {
                case -1:
                    cout << "fork() command not succseed\n" << endl;
                    exit(1);
                case 0:
                    if (execvp(args[0],args) == -1) {
                        cout << strerror(errno)<<endl;
                        cout << args[0] << " command not found" << endl;
                        exit(127);
                    }
                default:
                    if (deamonProc) {
                        cout << "[" << pid << "]\n";
                        lastExitStatus = 0;
                    }
                    else {
                        if (waitpid(pid, &status, 0) == -1) {
                            lastExitStatus = 1;
                            exit(1);
                        }
                        WEXITSTATUS_status = WEXITSTATUS(status);

                        if (WEXITSTATUS_status == 0)
                            lastExitStatus = 0;

                        else if (WIFSIGNALED(status)) {
                            WEXITSTATUS_status = 128 + WTERMSIG(status);
                            lastExitStatus = WEXITSTATUS_status;;
                            cout << "signal status:" << lastExitStatus << endl;
                        } else {
                            lastExitStatus = WEXITSTATUS_status;
                            cout << "exit status : " << lastExitStatus << endl;
                        }


                        while ((zombiePid = waitpid(-1, &status, WNOHANG)) > 0) {

                            int ZOMBIE_status = 0;
                            string zombieReturnStatus = "";

                            if (WIFEXITED(status)) {
                                ZOMBIE_status = WEXITSTATUS(status);
                                zombieReturnStatus = to_string(ZOMBIE_status);

                                cout << "deamon process : ["<< zombiePid << "] exit status : " << zombieReturnStatus << endl;


                            } else if (WIFSIGNALED(status)) {
                                ZOMBIE_status = 128 + WTERMSIG(status);
                                zombieReturnStatus = to_string(ZOMBIE_status);
                                cout << "deamon process : ["<< zombiePid << "] signal status : " << zombieReturnStatus << endl;

                            }

                            cout << "Process [" << zombiePid<< "] exited "<< endl;
                        }
                    }
            }

            return;

//                            while((zomb_pid=waitpid(-1,&pid_status,WNOHANG))>0)  // return the zomb_pid. "-1" means for any child process. &pid_status - the status of the pid.  WNOHANG means return immediately if not child has exited
//                            {
//                                int temp_status;
//
//                                if(WIFEXITED(pid_status)){   // WIFEXITED return true if the child terminated normally
//                                    temp_status=WEXITSTATUS(pid_status);
//                                    ss << temp_status;
//                                    zomb_status = ss.str();
//                                    cout << "exit status:" <<*exit_status<<endl;
//
//
//                                }
//
//                                else if(WIFSIGNALED(pid_status)){     // WIFSIGNALED - returned true if the child process was terminated by a signal
//                                    temp_status=128+WTERMSIG(pid_status);  //  128 is exit status when process terminated by signal + WTERMSIG(pid_status) ->the number of the signal that caused the child process
//                                    ss << temp_status;
//                                    zomb_status = ss.str();
//                                    cout << "signal status:" <<*exit_status<<endl;
//
//                                }
//
//                                else {
//                                    zomb_status = "0";
//                                }
//
//                                cout << '[' << zomb_pid << "]: exited,status=" << zomb_status << endl;
//
//                            }

            }
    }
}


string parseVar(string it){
    string parsedPath="";
    string varName;

    for(size_t i=0;i<it.length();i++)
    {
        if(it[i]=='$' && isCapitalLetter(it[i+1]))
        {
            i++;
                varName="";
                while(it[i] !='/' && i<it.length() && (isAlphaNumeric(it[i]) || it[i]=='_')) {
                        varName += it[i++];
                }
            if(isVariable(varName))
            {
                try {
                    char *varPath = getenv(varName.c_str());
                    if (varPath)
                        parsedPath += varPath;
                }
                catch (exception ex) {}
            } else
                perror("incorrect variable name\n");
            i--;
        }
        else if(it[i]=='$'&& it[i+1]=='?'){
            i++;
            parsedPath+=to_string(lastExitStatus);
        }
        else if (it[i]=='~')
        {
            parsedPath+=getenv("HOME");
        }

        else
            parsedPath+=it[i];

    }
    return parsedPath;
}

bool isVariable(string str){
    bool status=false;
    if(isCapitalLetter(str[0]))
    {
        for(size_t i=1;i<str.length();i++)
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



void runProccess(char **command){

}

void runBackProcess(char ** command){


}