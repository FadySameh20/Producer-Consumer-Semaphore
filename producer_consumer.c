#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#define BUFFER_SIZE 10
#define N 10


typedef struct Queue {
  int buffer[BUFFER_SIZE];
  int front; 
  int end;
} Queue;


Queue initialize();
int isEmpty();
int isFull();
void enqueue(int);
void dequeue();
void* count(void*);
void* monitor(void*);
void* collect(void*);
void checkThreadError(int);


sem_t mutexCounter, mutexBuffer, occupiedSlots, emptySlots;  //semaphores
int messagesCount = 0;  //shared variable between monitor & counter threads
Queue q;  //shared buffer (FIFO queue) between monitor and collector


int main() {
  srand(time(NULL));  //To generate different random numbers in each run
  
  //initializing semaphores
  sem_init(&mutexCounter, 0, 1);  //crtical section 1 
  sem_init(&mutexBuffer, 0, 1);  //critical section 2
  sem_init(&occupiedSlots, 0, 0);  //used to check empty buffer
  sem_init(&emptySlots, 0, BUFFER_SIZE);  //used to check full buffer
  
  initialize();  //initialize buffer (FIFO queue)
  
  pthread_t mCounter[N], mMonitor, mCollector;  //threads
  int counterThreadNum[N];
  int check;
  
  for(int i = 0; i < N; i++) {  //creating N mCounter threads
    counterThreadNum[i] = i+1;  //To assign a number to each thread
    check = pthread_create(&mCounter[i], NULL, count,&counterThreadNum[i]);
    checkThreadError(check);  //check failure in thread creation
  }
  
  check = pthread_create(&mMonitor, NULL, monitor, NULL);
  checkThreadError(check);
  
  check = pthread_create(&mCollector, NULL, collect, NULL);
  checkThreadError(check);
  
  //wait for all threads to finish there work brfore exectunig "return 0;"
  for(int i = 0; i < N; i++) {
    check = pthread_join(mCounter[i], NULL);
    checkThreadError(check);
  }
  
  check = pthread_join(mMonitor, NULL);
  checkThreadError(check);
 
  check = pthread_join(mCollector, NULL);
  checkThreadError(check);
  
  return 0;
}


Queue initialize() {  //initializing the queue
  q.front = -1;
  q.end = -1;
}


int isEmpty() {  //check if the queue is empty
  if(q.front == -1 && q.end == -1)
    return 1;
  else
    return 0;
}

int isFull() {  //check if the queue is full
  if((q.end + 1) % BUFFER_SIZE == q.front)
    return 1;
  else
    return 0;
}


void enqueue(int value) {  //inserting a new element in the queue
  if(isFull())
    return;
  else if (isEmpty()) {
    q.front = 0;
    q.end = 0;
  }
  else
    q.end = (q.end + 1) % BUFFER_SIZE;  //circular queue
  q.buffer[q.end] = value;
}


void dequeue() {
  if(isEmpty())
    return;
    
  if(q.front == q.end) {  //only one element left in the queue
    q.front = -1;
    q.end = -1;
  } else
      q.front = (q.front + 1) % BUFFER_SIZE;
}


void* count(void* arg) {  //function of N mcounter threads
  int* counterThreadNum = arg;  //number assigned to each mCounter thread
  
  while(1) {
    sleep(rand() % 4 + 1);  //activate at random time intervals
    printf("Counter thread %d: received a message\n", *counterThreadNum);
    printf("Counter thread %d: waiting to write\n", *counterThreadNum);
    
    sem_wait(&mutexCounter);  //mutual exclusion (critical section)
    messagesCount++;  //increment the shared variable
    printf("Counter thread %d: now adding to counter, counter value = %d\n\n", *counterThreadNum, messagesCount);
    sem_post(&mutexCounter);
  }
}


void* monitor(void* arg) {  //function of mMonitor thread
  int value, oldMessagesCount; 
  
  while(1) {
    sleep(rand() % 4 + 1);  //activate at random time intervals       
    printf("Monitor thread: waiting to read counter\n");
    
    sem_wait(&mutexCounter); //mutual exclusion on messagesCount
    oldMessagesCount = messagesCount;  //store the value to be used later
    printf("Monitor thread: reading a count value of %d\n", messagesCount);
    messagesCount = 0;  //reset the value of the shared variable
    sem_post(&mutexCounter);
    
    sem_getvalue(&emptySlots, &value);  //checking the semaphore value
    if(value == 0)  //sem_wait() will cause the thread to be blocked
      printf("Monitor thread: Buffer full !!\n\n"); //emptySlots=0 --> full
    sem_wait(&emptySlots);  //decrement the emptySlots
    
    sem_wait(&mutexBuffer);  //mutual exclusion on buffer    
    enqueue(oldMessagesCount);  //add the valueto the queue
    printf("Monitor thread: writing to buffer at position %d\n\n", q.end);
    sem_post(&mutexBuffer);
    
    sem_post(&occupiedSlots);  //increment occupiedSlots
  }
}


void* collect(void* arg) {  //function of mCollector thread
  int value, position;
  
  while(1) {
    sleep(rand() % 8 + 1);  //activate at random time intervals 
    
    sem_getvalue(&occupiedSlots, &value);  //checking the semaphore value
    if(value == 0)  //sem_wait() will cause the thread to be blocked
      printf("Collector thread: nothing is in the buffer!\n\n");  //full=0
    sem_wait(&occupiedSlots);  //decrement the occupiedSlots
    
    sem_wait(&mutexBuffer);  //mutual exclusion on buffer
    position = q.front;  //postion of the removed value 
    dequeue();  //remove a value from the queue
    printf("Collector thread: reading from buffer at position %d\n\n", position);
    sem_post(&mutexBuffer);
    
    sem_post(&emptySlots);  //increment the emptySlots
  }
}

void checkThreadError(int check) {  //check failure in pthread_create/join
  if(check != 0)  //On success it returns 0, otherwise error
  {
    printf("Error in creation of threads !");
    exit(-1);
  }
}
