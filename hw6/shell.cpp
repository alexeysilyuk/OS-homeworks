#include "shell.h"
#include <list>

using namespace std;
#define ARGSSIZE 1024
int lastExitStatus=0;

list< std::string > tokens;
struct pipeElement{
    int index;
    char direction;
    int pipe_identifier;
    string source,destination;
};

char **commands;
int commandIndex=0;

char buffer[ARGSSIZE+1];

string homedir;
string currentDir;

int main(void) {
    cout << "Welcome to \"@Shell\" " <<endl;
    // infinite REPL

    while(true){

        tokens.clear(); // remove old tokens
        cout << endl;
        cout << "@Shell: "<<getDir()<< "> ";

        read();
        evaluate();
    }
    return 1;
}


void read(){
    string line;

        cleanBuffer();  // clean std input stream
        getline(cin, line);

        // exit if terminated or choosen to exit
        if (cin.eof() || line.compare("exit") == 0)
        {
            cout << "Bye bye!" << endl;
            exit(lastExitStatus);
        }

    // run tokenization function
    tokenize(line);



}


// update global homedir and current dir
string getDir(){

    char buffer[1000];

        homedir = getenv("HOME");
        currentDir = getcwd(buffer, sizeof(buffer));

        // replace $HOME by ~ symbol
        if (currentDir.length() > homedir.length())
            currentDir = "~" + currentDir.substr(homedir.length());

        else if(currentDir.length() == homedir.length())
            currentDir = "~/";
        else
            currentDir = getcwd(buffer, sizeof(buffer)); // return the current directory

        return currentDir;
}

// separate user input to tokens
list<string> tokenize(string line){
    string buffer; // string buffer
    stringstream ss(line); // Insert the string into a stream

    // each separated by space token, push to tokens list
    while (ss >> buffer)
        tokens.push_back(buffer);

    return tokens;
}

// clean input buffer
void cleanBuffer(){
    cin.clear();							// clear error flags
    cin.ignore(cin.rdbuf()->in_avail());	// ask buffer for length of chars in it and clear all of them
}

//checking if there pipes in command, and push to list all pipes indexes with types
bool isPiped(){
    bool pipeFound=false;
    for (list<string>::const_iterator it = tokens.begin(), end = tokens.end(); it != end; ++it){

        std::size_t foundLeft = it->find("<");
        std::size_t foundRight = it->find(">");

        if (foundLeft!=std::string::npos)
        {
            pipeFound=true;
//            temp_pipe = new pipeElement();
//            temp_pipe->direction='l';
//            temp_pipe->index=i;
//            temp_pipe->pipe_identifier=atoi(it->substr(0,1).c_str());
//            pipes.push_back(*temp_pipe);
        }
        if (foundRight!=std::string::npos)
        {
            pipeFound= true;
//            temp_pipe = new pipeElement();
//            temp_pipe->direction='r';
//            temp_pipe->index=i;
//            temp_pipe->pipe_identifier=atoi(it->substr(0,1).c_str());
//            cout << temp_pipe->pipe_identifier<<endl;
//            pipes.push_back(*temp_pipe);
        }
    }

        return pipeFound;

}


//eval finction
void evaluate(){

    string path="";

//    bool piped = isPiped();
    int pipefd[2];
    bool piped=false;
    // loop iterates over tokens list
    for (list<string>::const_iterator it = tokens.begin(), end = tokens.end(); it != end; ++it)
    {

        // determine if it's PIPE
        std::size_t foundLeft = it->find("<");
        std::size_t foundRight = it->find(">");
        pipeElement *temp_pipe;

        if (foundLeft!=std::string::npos)
        {
            temp_pipe = new pipeElement();
            temp_pipe->direction='l';
            temp_pipe->pipe_identifier=atoi(it->substr(0,1).c_str());
            piped=true;
        }
        if (foundRight!=std::string::npos)
        {
            temp_pipe = new pipeElement();
            temp_pipe->direction='r';
            temp_pipe->pipe_identifier=atoi(it->substr(0,1).c_str());
            piped=true;
        }



        // if choosen CD command by user
        if(it->compare("cd")==0)
        {
            ++it;
            // check if have arguments
            if(it!=tokens.end())
            {
                // send token to parsing, replacing VARs by their values
                // after parsing path try to CHDIR
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
                    perror("@Shell: cd");
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
        // if choosen $? print last status
        else if (it->compare("$?")==0)
        {
            cout << lastExitStatus<<endl;
            return;
        }
        // choosen some other BASH command to execute
        else
        {
            int status; // for process status

            unsigned index = 0;
            unsigned toksNumber=0;
            int deamonProc=0;

            // iterate over list of tokens, check if DEAMON proccess needed
            // count real arguments amount for pending to execv
            for (list<string>::const_iterator iter = it; iter != tokens.end(); ++iter)
                {
                    if(*iter == "&" && index+1==tokens.size())
                        deamonProc=1;
                    else
                        toksNumber++;
                index++;
                }

            // allocate memory for args array+1 for NULL at the end of args list
            const char** array = new const char*[toksNumber+1];
            char *args[toksNumber+1];

            index=0;
            int subindex=0;
            pid_t  pidT;
            // because we need to send array of args to execv, need to copy
            // tokens from LIST type to char[]
            for (list<string>::const_iterator iter = it; iter != tokens.end(); ++iter) {

                std::size_t foundl = iter->find("<");
                std::size_t foundr = iter->find(">");

                if(*iter == "&" && index==toksNumber)
                    deamonProc=1;
                else
                {
                    if (foundl!=std::string::npos || foundr!=std::string::npos) {
//                        commands[commandIndex]=*args;
                        iter++;
                        pidT=fork();
                        int status;
                        if(pidT==0)
                        {
                            break;
                        }
                        else
                        {
//                            while(wait(&pidT)>0);
                            index=0;
                            cout << "father after fork"<<endl;
                        }


                    }


                    array[index] = iter->c_str();
                    args[index] = new char[ARGSSIZE];

                    // copy token after parsing it, if needed
                    strcpy(args[index], parseVar(array[index]).c_str());
//                    cout << args[index] << endl;
                    index++;


                }

            }
            // put NULL to end of args list
            args[index]=NULL;

            pid_t zombiePid,pid;
            int WEXITSTATUS_status;

            // fork processes
            switch(pid=fork()) {
                case -1:
                    cout << "fork() command not succseed\n" << endl;
                    exit(1);
                // for child process, run function execvp and send args list
                // in case of failure, print error and exit with status 127
                case 0:

                    if (execvp(args[0],args) == -1) {
                        cout << strerror(errno)<<endl;
                        cout << args[0] << " command not found" << endl;
                        exit(127);
                    }
                    break;

                // for parent process
                default:
                    if(piped){
                        close(pipefd[0]);
                        close(pipefd[1]);

                    }

                    if (deamonProc) {
                        cout << "[" << pid << "]\n";
                        lastExitStatus = 0;
                    }
                    // if not DEAMON, run WAITPID and wait for child process
                    // and update his exit status
                    else {
                        if (waitpid(pid, &status, 0) == -1)
                        {
                            lastExitStatus = 1;
                            exit(1);
                        }

                        // get child exit status
                        WEXITSTATUS_status = WEXITSTATUS(status);

                        // if child process succeed, update last status to 0
                        if (WEXITSTATUS_status == 0)
                            lastExitStatus = 0;

                        // if child process, closed with signal, update lastexitstatus
                        // with 128+signal number
                        else if (WIFSIGNALED(status)) {
                            WEXITSTATUS_status = 128 + WTERMSIG(status);
                            lastExitStatus = WEXITSTATUS_status;;
                            cout << "signal status:" << lastExitStatus << endl;
                        }
                        // if not terminated, update last exit status
                        else {
                            lastExitStatus = WEXITSTATUS_status;
                            cout << "exit status : " << lastExitStatus << endl;
                        }

                        // this loop carries ZOMBIE processes termination
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

            }
    }
}

// parse received var
string parseVar(string it){
    string parsedPath="";
    string varName;

    //iterate over token
    for(size_t i=0;i<it.length();i++)
    {
        // if detects '$' symbol and next symbol after it is capital, means VAR
        if(it[i]=='$' && isCapitalLetter(it[i+1]))
        {
            i++;
                // get variable name, seek '/' or string end for stop loop
                varName="";
                while(it[i] !='/' && i<it.length() && (isAlphaNumeric(it[i]) || it[i]=='_')) {
                        varName += it[i++];
                }
            // run function to check if variable is correct
            if(isVariable(varName))
            {
                //try to get variable from BASH environment
                try {
                    char *varPath = getenv(varName.c_str());
                    // if found VAR in env, cancatinate value to parsedPath
                    if (varPath)
                        parsedPath += varPath;
                }
                    // if VAR not exists, skip it
                catch (exception ex) {}
            }
                // if varible in incorrect
            else
                perror("incorrect variable name\n");
            i--;
        }
        // if found '$?' cancatinate lastexit status to parsedPAth
        else if(it[i]=='$'&& it[i+1]=='?'){
            i++;
            parsedPath+=to_string(lastExitStatus);
        }
        // if found '~', cancatinate home directory to parsedPath
        else if (it[i]=='~')
        {
            parsedPath+=getenv("HOME");
        }
        //in all other cases, simple cancatinate character as is
        else
            parsedPath+=it[i];

    }
    return parsedPath;
}

// chech if variable consists allowed structure, starts with capital letter,
//contains only alphanumeric or '_' symbol
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


