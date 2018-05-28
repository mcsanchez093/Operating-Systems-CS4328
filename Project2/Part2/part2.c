/* Author: Maria Sanchez 
   Note: Code does not compile on macOS 
   because it does not support unnamed semaphores. 
   Compiles on Linux Servers.
*/

#include <pthread.h> /* Needed for pthreads */ 
#include <time.h>	/* Needed for rand */
#include <unistd.h>		/* Needed for sleep() */		
#include <semaphore.h>	/* Needed for semaphore */
#include <stdlib.h>				
#include <stdio.h>	/* Input/output */		
#include <signal.h>

pthread_t *Students;		/* N threads running as Students. */
pthread_t TA;				/* Separate Thread for TA. */

int ChairsCount = 0;
int CurrentIndex = 0;

/* Declaration of Semaphores and Mutex Lock. */
sem_t TA_Sleep;
sem_t Student_Sem;
sem_t ChairsSem[3];
pthread_mutex_t ChairAccess;

/* Declared Functions */
void *TA_Activity();
void *Student_Activity(void *threadID);
/* void alarmHandler */

void alarmHandler(int sig){
	raise(SIGTERM);
}

int main(int argc, char* argv[])
{
	signal(SIGALRM, alarmHandler);
	int number_of_students;		
	int seconds;
	int id;
	srand(time(NULL));

	/* Initializing Mutex Lock and Semaphores.*/
	sem_init(&TA_Sleep, 0, 0);
	sem_init(&Student_Sem, 0, 0);
	for(id = 0; id < 3; ++id)			
		sem_init(&ChairsSem[id], 0, 0);

	pthread_mutex_init(&ChairAccess, NULL);
	/* Validate user input from command line */
	if(argc !=3){
		printf("Usage: ./part2 #seconds #students \n");
   		exit(1);
	}

	/* If not specified, default value for number of students is set to 5 and that of time is 30 seconds.
 * 	if(argc!= )
 * 		{
 * 				printf("Default number of students taken.\n");
 * 						number_of_students = 5;
 * 								seconds = 30;
 * 									}
 *
 * 										else if(argc < 3){
 * 												seconds = atoi(argv[1]);
 * 														number_of_students = 5;
 * 															} */

	
	/* If the number have been specifies */
	else
	{
		/* printf("Number of Students specified. Creating %d threads.\n", number_of_students); */
		seconds = atoi(argv[1]);
		number_of_students = atoi(argv[2]);
	}
		
	alarm(seconds);
	/* Allocate memory for Students */
	Students = (pthread_t*) malloc(sizeof(pthread_t)*number_of_students);

	/* Creating TA thread and N Student threads. */
	pthread_create(&TA, NULL, TA_Activity, NULL);	
	for(id = 0; id < number_of_students; id++)
		pthread_create(&Students[id], NULL, Student_Activity,(void*) (long)id);

	/* Waiting for TA thread and N Student threads. */
	pthread_join(TA, NULL);
	for(id = 0; id < number_of_students; id++)
		pthread_join(Students[id], NULL);

	/* Free allocated memory */
	free(Students); 
	return 0;
}

void *TA_Activity()
{
	while(1)
	{
		sem_wait(&TA_Sleep);		/* TA is currently sleeping.*/
		printf("\n");
		printf("TA is woken up... grumble...\n");

		while(1)
		{
			/* lock */
			pthread_mutex_lock(&ChairAccess);
			if(ChairsCount == 0) 
			{
				/* if chairs are empty, break the loop.*/
				pthread_mutex_unlock(&ChairAccess);
				break;
			}
			/* TA gets next student on chair. */
			sem_post(&ChairsSem[CurrentIndex]);
			ChairsCount--;
			printf("Student left his/her chair. Remaining Chairs %d\n", 3 - ChairsCount);
			CurrentIndex = (CurrentIndex + 1) % 3;
			pthread_mutex_unlock(&ChairAccess);
			/* unlock */

			/*printf("\t TA is currently helping the student.\n");*/
			sleep(5);
			sem_post(&Student_Sem);
			usleep(1000);
		}
	}
}

void *Student_Activity(void *threadID) 
{
	int ProgrammingTime;

	while(1)
	{
		printf("Student %ld is doing programming assignment.\n", (long)threadID);
		ProgrammingTime = rand() % 10 + 1;
		sleep(ProgrammingTime);		/* Sleep for a random time period. */
		printf("Student %ld needs help from the TA\n", (long)threadID);
		
		pthread_mutex_lock(&ChairAccess);
		int count = ChairsCount;
		pthread_mutex_unlock(&ChairAccess);

		if(count < 3)	/*Student tried to sit on a chair.*/
		{
			if(count == 0)	/*If student sits on first empty chair, wake up the TA.*/
				sem_post(&TA_Sleep);
			else
				printf("Student %ld takes a seat in the hall. \n", (long)threadID);

			/* lock*/
			pthread_mutex_lock(&ChairAccess);
			int index = (CurrentIndex + ChairsCount) % 3;
			ChairsCount++;
			printf("\n");
			printf("Students waiting: %d\n", ChairsCount);
			pthread_mutex_unlock(&ChairAccess);
			/* unlock */

			sem_wait(&ChairsSem[index]);		/* Student leaves his/her chair.*/
			printf("\n");
			printf("\t TA is helping Student %ld. \n", (long)threadID);
			sem_wait(&Student_Sem);		/* Student waits to go next. */
			printf("Student %ld left TA room.\n",(long)threadID);
		}
		else 
			printf("\n");
			printf("Student %ld goes back to program. \n", (long)threadID);
			/*If student didn't find any chair to sit on.*/
	}
}

