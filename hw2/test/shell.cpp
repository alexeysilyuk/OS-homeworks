
#include "shell_functions.h"


const int MAX_ARGS = 1024;



int main(int argc,char *args[]){
	//variables declerations
	string command;
    char *argv[MAX_ARGS],     	
    	 space[]=" ";	       // the seperate value to split  string

    int num_args=0;
    string exit_status="0";

	cout<<"Welcome to OS SHell!\n\n";

	while(true)
	{

		try{ 

			getCurrentDirectory();
			getline(cin,command); // get the command from the user

			if(command =="exit" || cin.eof()) 					// if the user enter exit word or Ctrl+D
			{
				for(int i=0;i<num_args;i++){
					delete [] argv[i];
				}

				cout<< "C ya!"<<endl;
				return 0;
			}
	        num_args = split_str(command,argv, space); 			// return to the argc array the each word in each cell like: cd ../labs => ['c' , '../labs']

	        if(num_args>0)
	        {
	        	interpt_str(argv,num_args, &exit_status);
	        	
	        }
	    }

	    catch(const invalid_argument& ia){ 						// invalid argument exception 
	    	cout<<"status " << ia.what()<<endl;
	    }
	    catch(...){
	    	cout<< "ERROR: some error accured."<<endl;
	    }
	} 
	return 0;
}





