//
// Created by alex on 4/13/18.
//

#ifndef HW3_SEMAPHORE_H
#define HW3_SEMAPHORE_H
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <wait.h>



////////////////////////////////////////////////////////////////
////// THIS CODE BEEN TAKEN FROM LAB5 EXAMPLE AND CHANGED //////
////////////////////////////////////////////////////////////////

#define SEMPERM 0600

int initsem(key_t semkey, int initval)
{

    union semun
    {
        int val;
        struct semid_ds *stat;
        ushort *array;
    } ctl_arg;

    int status = 0, semid;

	if ( ( semid = semget(semkey, 1,  IPC_CREAT | IPC_EXCL | SEMPERM) ) == -1 )
	{
		if( errno ==  EEXIST )
		{
			if ( (semid = semget(semkey, 1, 0) ) == -1)
			{
				printf("Error creating semaphores\n");
				exit(0);
			}
		}
		else
		{
			printf("Error creating semaphores\n");
			exit(0);
		}

	}

	ctl_arg.val = initval;

	/*
	semctl() changes permissions and other characteristics of a semaphore set. It is prototyped as follows:
	int semctl(int semid, int semnum, int cmd, union semun arg);
	The semnum value selects a semaphore within an array by its index.
	The cmd argument is one of the control flags.

	SETVAL
	-- Set the value of a single semaphore. In this case, arg is taken as arg.val, an int.
	*/

	status = semctl(semid, 0, SETVAL, ctl_arg);

	if(status == -1)
	{
		perror("initsem failed");
		exit(0);
	}

	return semid;
}

int p(int semid)
{
    struct sembuf sops;
    sops.sem_num = 0;
    sops.sem_op = -1;
    sops.sem_flg = SEM_UNDO;

    if (semop(semid,&sops,1)==-1)
    {
        perror("p(semid) failed");
        exit(1);
    }
    else
        return 0;

}

void v(int semid)
{
    struct sembuf sops;
    sops.sem_num = 0;
    sops.sem_op = 1;
    sops.sem_flg = SEM_UNDO;

    if (semop(semid,&sops,1)==-1)
    {
        perror("v(semid) failed");
        exit(1);
    }
    else
        return;


}
#endif //HW3_SEMAPHORE_H
