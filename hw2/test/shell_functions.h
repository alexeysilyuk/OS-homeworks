#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdexcept>
#include <string>     
#include <signal.h>
#include <string.h>
#include <sstream>


#define COMMAND_SUCCESS 0
#define COMMAND_FAILED  1
#define STATUS_EXIT   127




using namespace std;

void catch_ctrlc(int sig_num)
{
 // flag = 1;
}

void getCurrentDirectory(){
    string  current_directory, // the current directory (the all), like: home/adir/desktop/...
            home_directory;    // only the directory of the home until Desktop, like: home/adir
    char cwd[1024];         // for input the directory
    home_directory=getenv("HOME"); //desktop/adirzoari
    current_directory = getcwd(cwd,sizeof(cwd)); // return the current directory 
        //current_directory="~"+current_directory.substr(home_directory.length()); // change the display of the path '~' replace the home
    if(current_directory.length()>home_directory.length()){ // change the display of the path '~' replace the home strcat(current_directory,"~");
        current_directory="~"+current_directory.substr(home_directory.length());
    }
    else    
        current_directory = getcwd(cwd,sizeof(cwd)); // return the current directory 

    cout <<"OS SHell:"+current_directory +">";
}


int split_str(string input, char **argv , char *delimiter){
    char cmd[100], *tok;
    int i;
    if (input.empty()){return 0;}
    // str.c_str() - convert the string value to char* value
    strcpy(cmd, input.c_str()); 
    i = 0;
    // strtok(str, del) // return the array of words which seprerated by the value of delimiter string
    tok = strtok(cmd , delimiter);
    while(tok) // move on the all words in tokens
    {
        argv[i] = (char*)malloc(strlen(tok)+1);  // malloc to size of word + 1 char place to '\0'
        strcpy(argv[i++], tok); // copy the current word in tok , to the array words argv
        tok = strtok(NULL, delimiter); // keep to the next word
    }
    argv[i] = NULL;
    return i;                                                   // return the amount of word the that now tokkened     
}


void parser_to_string(char **directory,int num_args,string *exit_status){

    string realDirectory="";
    unsigned int len;
    char *words[1024], delimeter[] = "/";
   


    
    len = split_str(*directory, words, delimeter);

    for(unsigned int i=0;i<len;i++) // each word   
    {
        for(unsigned int j=0;j<strlen(words[i]);j++){             // each character in the word
   

            if(j<strlen(words[i])-1 && words[i][j]=='$' && words[i][j+1]=='?' )
            {           
                realDirectory+=*exit_status;
                j++;
            }


            else if(words[i][j]=='~')
            {    
                char *env_home=getenv("HOME");              
                realDirectory+=env_home;
            }

            

            else if (j<strlen(words[i])-1 && words[i][j]=='$' && ( (words[i][j+1]>='A' && words[i][j+1]<='Z') || (words[i][j+1]>='a' && words[i][j+1]<='z') || (words[i][j+1]=='_') ))  /// check if it $enviroment variable like $HOME
            {
                string env_string="";
                while((j<strlen(words[i])-1 && words[i][j+1]>='A' && words[i][j+1]<='Z') || (words[i][j+1]>='a' && words[i][j+1]<='z') || (words[i][j+1]=='_' )  || (words[i][j+1]>='0' && words[i][j+1]<='9') ) 
                {
                    env_string+=words[i][j+1];
                    j++;
                }
                if(getenv(env_string.c_str())==NULL)
                {           // if the argument is not environment variable it throws execption
                    *exit_status="1";
                     throw invalid_argument("Invalid Argument: "+ env_string +" is not environment variable");
                    //perror(env_string.c_str() +"is not environment variable");
                }
                else 
                { 
                    realDirectory+=getenv(env_string.c_str());

                }


            }

            
            else 
            { 
                realDirectory+=words[i][j];
            } 

            
        }
        if(i < len - 1)
           realDirectory += "/";
    }
    
    if(*directory != NULL){
        delete [] *directory;
    }
    *directory = (char*) malloc(realDirectory.length() + 1);
    strcpy(*directory,realDirectory.c_str());


    
}



void interpt_str(char **argv,int num_args,string *exit_status){ 

	if(strcmp(argv[0], "cd") == 0)
    {   
			if(num_args > 2)
            {
                *exit_status="1";
                perror("Too many parameters for cd command");


            }
         
            
            else if(argv[1]!=NULL)
            {
                parser_to_string(&argv[1],num_args,exit_status);
            }
            else{  // if enter only cd
                return ;
            }

            if(chdir(argv[1]) != 0)  {
                *exit_status=1; // set the exit status to 1 (directory not found)
                char error_msg[104]="cd: ";
                strcat(error_msg,argv[1]);
                perror(error_msg); 
               
            }
            else{
                *exit_status="0";
                char cwd[1024];  
                string current_directory = getcwd(cwd,sizeof(cwd));  // get the current directory 
            }
    }
    else if(strcmp(argv[0], "$?") == 0){
            cout<<"$?:-->  ";
            cout<<*exit_status<<endl;
    }


    // if not cd command

    else{

        int pid_status=0,        // pid status
                rec_status=0;    // status return from WEXITSTATUS
        bool amperbool=false;    // set to true if last command is &   
        stringstream ss;         // for convert int to string -> used by stringstream



        
        for(int i = 0; i < num_args; i++){
            parser_to_string(&argv[i],num_args,exit_status);
        }
        

        
        if(strcmp(argv[num_args-1],"&")==0)
        {   

            amperbool=true;
            char **lessAmp= (char**)malloc(num_args-1);  // malloc to size of word + 1 char place to '\0'
            for(int i = 0; i < num_args; i++){
                if(i<num_args-1)
                {
                    lessAmp[i]=(char*)malloc(strlen(argv[i])+1);
                    strcpy(lessAmp[i],argv[i]);
                }
                delete [] argv[i];
            }
            argv = lessAmp;


        }
        


        
        int zomb_pid;       // zombie id
        string zomb_status; // status of zombie

        int pid = fork();   // create son process


        if(pid < 0)  // if fork failed
        {
            perror("Error function fork() failed");
        }

        if(pid==0) // son process
        {
          
            if(execvp(argv[0],argv)==-1)
            {
                cout<< argv[0] <<" command not found" <<endl;
                exit(127); 
                return;
            }
            cout << "This line should never showed! REMEMBER , Its after execvp command!!!"<<endl;
        }


        else     // parent process 
        {
            if(amperbool){                      // if amperbool==true means & in the last argument -> waitpid() is not called, and last exit status is reset to 0.
                
                cout << '[' << pid << ']'<<endl;  // id process
                *exit_status="0";               // commannd success
            }

            else   // amperbool==false
            {

                if(waitpid(pid,&pid_status,0) == -1)  // waitpid get pid-process son id , status and options. the parent process wait until his son process finished 
                {
                    *exit_status="1";
                    exit(1);
                }

                rec_status=WEXITSTATUS(pid_status); // return the exit status of the child process

                if(rec_status==0)                 //  successful set to ZERO
                    *exit_status="0"; 


                
                else if(WIFSIGNALED(pid_status))    // WIFSIGNALED - returned true if the child process was terminated by a signal
                {
                    rec_status=128+WTERMSIG(pid_status);  //  128 is exit status when process terminated by signal + WTERMSIG(pid_status) ->the number of the signal that caused the child process
                    ss<<rec_status;                // ss is stringstream-for convert int to string
                    *exit_status=ss.str();     
                    cout << "signal status:" <<*exit_status<<endl;    
                }

                else                        // rec_status > -->command not succesful "throw" any other status or execvp failed
                {
                  
                    ss<<rec_status;         // input the ss to rec_status
                    *exit_status=ss.str();  //convert to string
                    cout<<"exit status:--->"<<*exit_status<<endl;
                }

            }
 
            while((zomb_pid=waitpid(-1,&pid_status,WNOHANG))>0)  // return the zomb_pid. "-1" means for any child process. &pid_status - the status of the pid.  WNOHANG means return immediately if not child has exited
            {
                int temp_status;

                if(WIFEXITED(pid_status)){   // WIFEXITED return true if the child terminated normally 
                    temp_status=WEXITSTATUS(pid_status);
                    ss << temp_status;
                    zomb_status = ss.str();
                    cout << "exit status:" <<*exit_status<<endl;    


                }

                else if(WIFSIGNALED(pid_status)){     // WIFSIGNALED - returned true if the child process was terminated by a signal
                    temp_status=128+WTERMSIG(pid_status);  //  128 is exit status when process terminated by signal + WTERMSIG(pid_status) ->the number of the signal that caused the child process
                    ss << temp_status;
                    zomb_status = ss.str();
                    cout << "signal status:" <<*exit_status<<endl;    

                }

                else {
                      zomb_status = "0";
                }

                cout << '[' << zomb_pid << "]: exited,status=" << zomb_status << endl;

            }


        }
        
        
	}

    
}



