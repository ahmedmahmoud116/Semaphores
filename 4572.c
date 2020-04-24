#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
//#define size_of_buffer 1 //here to control size of buffer

void * counter(void* u);  /* the three threads */
void * monitor(void* u);  /* producer */
void * collector(void * u); /* consumer */

int count = 0;
sem_t mcounter, empty, full, s;

int *buffer;
int size_of_buffer;

int last_index = 0;

void * counter(void * u){
    int index = (int)u;
    sleep(rand()%10); // to wake the counter
    printf("Counter thread %d: received a message\n",index);
   // sleep(1);

    if(sem_trywait(&mcounter) == -1){ //to check if the semaphore is +ve then lock the semaphore and continue if not print the statment and lock the semaphore
        // sleep(1);
        printf("Counter thread %d: waiting to write\n",index);
        sem_wait(&mcounter);
    }
        sleep(1);
        count++; //it only increase the counter
        printf("Counter thread %d: now adding to counter,counter value = %d\n",index,count);
        sem_post(&mcounter);
}

void * monitor(void* u){

   for(int i = 0;i < (int)u; i++){  //to generate monitor and consumers with limit

    sleep(rand()%10);
    if(sem_trywait(&mcounter) == -1){
        // sleep(1);
        printf("Monitor thread: waiting to read counter\n");
        sem_wait(&mcounter);
    }
    sleep(1);    //sleeping in critical section just to give sometimes to print in sequence
    int m = count; //taking the valye of counter in local variable to put it in the buffer in another critical section
    printf("Monitor thread: reading a count value of %d\n",count);
    count = 0; //reset the counter
    sem_post(&mcounter);

    /***part 2 to put in the buffer1***/
    if(sem_trywait(&empty) == -1){
        // sleep(1);
        printf("Monitor thread: Buffer full!!\n");
        sem_wait(&empty);
    }
    if(sem_trywait(&s) == -1){
        // sleep(1);
        printf("Monitor thread: waiting to write to buffer\n");
        sem_wait(&s);
    }
    for(int i = 0;i < size_of_buffer;i++){ //scan the array till find an empty space to put in it
        if(buffer[i] == 0){ //if find and empty space
            sleep(1); //sleeping just to give sometimes to print in sequence
            printf("Monitor thread: writing to buffer at position %d\n",i);
            buffer[i] = m;
            break;
        }
    }
    sem_post(&s);
    sem_post(&full);

    }
}


void * collector(void * u){
   for(int i = 0;i < (int)u; i++){

    sleep(rand()%10);

    if(sem_trywait(&full) == -1){ //to check if the semaphore is +ve then lock the semaphore and continue if not print the statment and lock the semaphore
        // sleep(1);
        printf("Collector thread: nothing is in the buffer!\n"); //if full = 0
        sem_wait(&full);
    }
    if(sem_trywait(&s) == -1){
        // sleep(1);
        printf("Collector thread: waiting to collect from the buffer!\n");
        sem_wait(&s);
    }
    int n = size_of_buffer; // to continue from last index to not do starving
    for(int i = last_index; i<size_of_buffer;i++){
        if(n == 0){
            break;
        }
        if(buffer[i] != 0){
            printf("Collector thread: reading from buffer at position %d\n",i);
            buffer[i] = 0;
            last_index = i;
            break;
        }
        if(i == size_of_buffer - 1){
            i = 0;
        }
        n--;
    }
    sem_post(&s);
    sem_post(&empty);

    }
}


int main()
{
    int ncounters, limit;

    printf("Enter the number of counters: ");
    scanf("%d",&ncounters);
    printf("Enter the size of Buffer: ");
    scanf("%d",&size_of_buffer);
    buffer = malloc(size_of_buffer * sizeof(int));
    printf("Enter the limit of interrupts of monitor and collector: ");
    scanf("%d",&limit);
    printf("\n");

    /***initialization of semaphores***/
    sem_init(&mcounter, 0, 1); //semaphores for counter
    sem_init(&empty, 0, size_of_buffer);    //semaphores for empty places in the buffer
    sem_init(&full, 0, 0);     //semaphores for full spaces in the buffer
    sem_init(&s, 0, 1);     //semaphores for critical section in the buffer


    pthread_t mcounters[ncounters];  //threads array of mcounter
    pthread_t mmonitor;       //monitor thread
    pthread_t mcollector;     //monitor collector

    for(int i = 0;i < ncounters ; i++){ //to generate number of counters
        pthread_create(&mcounters[i],NULL,&counter,i);
    }

        pthread_create(&mmonitor,NULL,&monitor,limit);
        pthread_create(&mcollector,NULL,&collector,limit);

        pthread_join(mmonitor,NULL);    //waiting for monitor and collector to just generate only one of them
        pthread_join(mcollector,NULL);

    for(int i = 0;i < ncounters ; i++){
        pthread_join(mcounters[i],NULL);
    }
    printf("\n\n%d",count);

    return 0;
}
