#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>

#define BRIDGE 0
#define TICKETS 1
#define FERRY 2
#define DOOR_BEACH 3
#define DOOR_FERRY 4
#define TRIP_START 5
#define TRIP_ENDED 6
#define TRIPS 10

struct sembuf sops[1];

void semb_op(struct sembuf* semop, int sem_num, int sem_op)
{
	semop->sem_num = sem_num;
	semop->sem_op  = sem_op;
	semop->sem_flg = 0;
}

int trip(int num, int semid)
{
	int i = 0;
	while (semctl(semid, TRIPS, SEM_INFO) < 0)				//for (int i = 0; i < TRIPS; i++)
	{
		if(i)
		{
			semb_op(sops, TRIP_ENDED, 0);					//waiting for ending previous trip
			semop(semid, sops, 1);
		}
	
		semb_op(sops, TICKETS, -1);							//buying tickets
		if (semop(semid, sops, 1) < 0)
			break;
		printf("Passenger #%d bought a ticket\n", num);

		semb_op(sops, DOOR_BEACH, 0);						//waiting for opening way to bridge
		semop(semid, sops, 1);

		semb_op(sops, BRIDGE, -1);							//decreasing free space on bridge
		semop(semid, sops, 1);
		printf("Passenger #%d on the bridge\n", num);

		semb_op(sops, DOOR_FERRY, 0);						//waiting for opening way to ferry
		semop(semid, sops, 1);

		semb_op(sops, FERRY, -1);							//decreasing free space on ferry
		semop(semid, sops, 1);
		semb_op(sops, BRIDGE, 1);							//increasing free space on bridge
		semop(semid, sops, 1);
		printf("Passenger #%d on ferry\n", num);

		semb_op(sops, TRIP_START, 0);						//waiting for start of the trip
		semop(semid, sops, 1);

		semb_op(sops, DOOR_FERRY, 0);						//waiting for opening way on bridge back to beach
		semop(semid, sops, 1);

		semb_op(sops, FERRY, 1);							//increasing free space on ferry
		semop(semid, sops, 1);
		semb_op(sops, BRIDGE, -1);							//decreasing free space on bridge
		semop(semid, sops, 1);
		printf("Passenger #%d on the bridge\n", num);

		semb_op(sops, DOOR_BEACH, 0);						//waiting for opening way to beach (finising trip)
		semop(semid, sops, 1);

		semb_op(sops, BRIDGE, 1);							//increasing free space on bridge 
		semop(semid, sops, 1);
		semb_op(sops, TICKETS, 1);							//increasing available amount of tickets for next trip
		semop(semid, sops, 1);		
		printf("Passenger #%d on the beach\n", num);

		semb_op(sops, TRIP_ENDED, -1);						//signaling that passenger ended his trip and he is on the beach
		semop(semid, sops, 1);		

		i++;
	}
}

void boat(int semid, int m, int n)
{
	for (int i = 0; i < TRIPS; i++)
	{
		semb_op(sops, TICKETS, 0);							//waiting for selling all tickets
		semop(semid, sops, 1);
		
		semb_op(sops, DOOR_BEACH, -1);						//opening way to bridge
		semop(semid, sops, 1);

		semb_op(sops, BRIDGE, 0);							//waiting for filling all space on bridge
		semop(semid, sops, 1);

		semb_op(sops, DOOR_FERRY, -1);						//opening way to ferry
		semop(semid, sops, 1);

		semb_op(sops, FERRY, 0);							//waiting for all passengers
		semop(semid, sops, 1);

		semb_op(sops, DOOR_FERRY, 1);						//closing way to ferry
		semop(semid, sops, 1);
		semb_op(sops, DOOR_BEACH, 1);						//closing way to beach
		semop(semid, sops, 1);

		semb_op(sops, TRIP_START, -1);						//signaling that trip started
		semop(semid, sops, 1);

		sleep(1);											//trip
		printf("Trip#%d\n", i + 1);

		semb_op(sops, TRIP_START, 1);						//setting semaphore for next trip
		semop(semid, sops, 1);
	
		semb_op(sops, DOOR_FERRY, -1);						//opening way to bridge (ending the trip)
		semop(semid, sops, 1);
		semb_op(sops, DOOR_BEACH, -1);						//opennig way to beach (ending the trip)
		semop(semid, sops, 1);

		semb_op(sops, TRIP_ENDED, 0);						//waiting for all passengers (waiting for m signals that they are on the beach)
		semop(semid, sops, 1);
		semb_op(sops, TRIP_ENDED, m);						//setting value for next trip
		semop(semid, sops, 1);

		semb_op(sops, DOOR_FERRY, 1);						//closing way to ferry
		semop(semid, sops, 1);
		semb_op(sops, DOOR_BEACH, 1);						//closing way to beach
		semop(semid, sops, 1);
	}

	semctl(semid, 0, IPC_RMID);								//removing semaphores
}

int main(int argc, char* argv[])
{
	pid_t pid;
	int n = atoi(argv[1]); // number of passengers on land
	int k = atoi(argv[2]); // max number of passengers on bridge
	int m = atoi(argv[3]); // max number of passengers on ferry

	int semid = semget(IPC_PRIVATE, 7, IPC_CREAT|IPC_EXCL|0666);
	
	if (semid < 0){
		perror("Error with creating semophore");
		exit(EXIT_FAILURE);
	}

	if (semctl(semid, BRIDGE, SETVAL, k) < 0){ //setting initial values for semaphores
		perror("semctl bridge");
		exit(EXIT_FAILURE);
	}

	if (semctl(semid, TICKETS, SETVAL, m) < 0){//setting initial values for semaphores
		perror("semctl tickets");
		exit(EXIT_FAILURE);
	}

	if (semctl(semid, FERRY, SETVAL, m) < 0){//setting initial values for semaphores
		perror("semctl ferry");
		exit(EXIT_FAILURE);
	}

	if (semctl(semid, DOOR_BEACH, SETVAL, 1) < 0){//setting initial values for semaphores
		perror("semctl door beach");
		exit(EXIT_FAILURE);
	}
	
	if (semctl(semid, DOOR_FERRY, SETVAL, 1) < 0){//setting initial values for semaphores
		perror("semctl door ferry");
		exit(EXIT_FAILURE);
	}

	if(semctl(semid, TRIP_START, SETVAL, 1) < 0){//setting initial values for semaphores
		perror("semctl trip start");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < n; i++)
	{
		pid = fork();							//creating passenger
		
		if (pid == 0)
		{
			trip(i, semid);	
			return 0;
		}
	}

	boat(semid, m, n);							//managing the trip

	for (int i = 0; i < n; i++)
		wait(NULL);	

	printf("All trips are finished\n");

}
