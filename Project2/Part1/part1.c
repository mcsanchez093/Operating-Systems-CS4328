#include <pthread.h> /* Needed for pthreads */
#include <semaphore.h> /* Needed for semaphores */
#include <time.h> /* Needed to generate random numbers */
#include <stdio.h> /* Needed for printf() and others */
#include <stdlib.h> /* Needed for exit() */

/* Declare global buffer */
#define BUFFER_SIZE 10

/* Declare integer array */
int buffer[BUFFER_SIZE];

/* Create a pthread mutex */
pthread_mutex_t mutex;

/* Declare semaphores */
sem_t full; 
sem_t empty;  

/* Logical pointers */
int in = 0; /* Points to the next free position  in the buffer */
int out = 0; /* Points to the first full position in the buffer */

/*  Will inrt a random generated integer(between 0 and 1000) to the buffer */
void *insert_item(void *param){
  /* Declare variables */
  int random;
  int item = *((int*)param);
  /* Seed needed to generate random numbers */
  srand(time(NULL));
  /* Critical section */
  do{
    /* Entry Section */
    /* Generate random numbers */
    random = rand() % 1000;
    /* Semaphore waits to get empty */
    sem_wait(&empty);
    pthread_mutex_lock(&mutex);


    /* Critical Section */
    /* Generate randoms numbers */
    buffer[in] = random;
    printf("The Producer %d produced: %d\n", item, buffer[in]);
    in = (in + 1) % BUFFER_SIZE;

    /* Unlocks the mutex */
    pthread_mutex_unlock(&mutex);
    sem_post(&full);

  }while(1);
}

void *remove_item( void * param){
  int item = *((int*)param);
  do{
    sem_wait(&full);

    pthread_mutex_lock(&mutex);
    int consumed = buffer[out];
    printf("\tThe Consumer %d consumed: %d\n", item, buffer[out]);
    out = (out + 1) % BUFFER_SIZE;


    /* Unlocks the mutex */
    pthread_mutex_unlock(&mutex);
    sem_post(&empty);

  }while(1);
}

int main(int argc, char *argv[]){
  /* Validate command line arguments */
  if(argc !=4){
    printf("Usage: ./part1 sleeptime num_producers num_consumers \n");
    exit(1);
  }
  /* Get command line arguments */
  const long int sleeptime = strtol(argv[1], NULL, 0);
  const long int num_producer = strtol(argv[2], NULL, 0);
  const long int num_consumer = strtol(argv[3], NULL, 0);

  /* Check for valid input, positive integer only */
  if (sleeptime < 0 || num_producer< 0 || num_consumer < 0){
    printf("Enter only positive values in arguments!!");
    return 1;
  }
 
  /* Initialize with sem_init the buffer, mutex, semaphores, and other global vars */
  pthread_mutex_init(&mutex, NULL);
  sem_init(&empty, 0, BUFFER_SIZE);
  sem_init(&full, 0, 0);
  int total = num_consumer + num_producer;
  /* Variables needed */
  int i, num_threads, num;

  /* Create producer and consumer thread(s) */
  pthread_t threads[total]; 

 for(i = 0; i < num_producer; i++){
    num = i;
    num_threads = i;
    pthread_create(&threads[num], NULL, insert_item, (void *) &num_threads);
  }  
  for ( i = num_producer; i< total; i++){
    num = i;
    num_threads = i - num_producer;
    pthread_create(&threads[num], NULL, remove_item, (void *) &num_threads);
  }

  /* Sleep for the sleeptime entered by user */
  sleep(sleeptime); 

  for (i = 0; i < total; i ++){
    pthread_cancel(threads[i]);
  }
  return 0;
}


