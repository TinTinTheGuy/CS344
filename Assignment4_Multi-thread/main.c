#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define SIZE 11 

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t myCond1, myCond2 = PTHREAD_COND_INITIALIZER; 

int myCount; 
bool myHold; 

void Producerloop() {
  pthread_mutex_lock(&myMutex);

  while (myCount < SIZE-1) {
    if(!myHold) {
      printf("PRODUCER: myMutex locked\n");
      myCount++; // Increase myCount if it's not 10 yet
      myHold = true;
      printf("PRODUCER: myMutex unlocked\n");
      printf("PRODUCER: signaling myCond2\n");
      pthread_cond_signal(&myCond2);
    } else {
      printf("PRODUCER: waiting on myCond1\n");
      pthread_cond_wait(&myCond1, &myMutex);
    }
  }
  pthread_mutex_unlock(&myMutex);
}

void* Consumerloop() {
  pthread_mutex_lock(&myMutex);
  while (1) {
    if (myCount <= 0 || myHold == false) {
      printf("CONSUMER: waiting on myCond2\n");
      pthread_cond_wait(&myCond2, &myMutex);
      pthread_mutex_unlock(&myMutex);
      continue;
    }
    printf("CONSUMER: myMutex locked\n");
    printf("myCount: %d -> %d\n", myCount-1, myCount); // Print statement
    myHold = false;
    printf("CONSUMER: myMutex unlocked\n");
    pthread_mutex_unlock(&myMutex);
    if (myCount == 10) {
      return NULL;
    }

    printf("CONSUMER: signaling myCond1\n");
    pthread_cond_signal(&myCond1);
  }

  return NULL;
}

int main(int argc, char* argv[]) {
  printf("PROGRAM START\n");
  pthread_t conThread; // Consumer thread
  pthread_cond_init(&myCond1, NULL);
  pthread_cond_init(&myCond2, NULL);
  myCount = 0; // Set myCount to 0 at the start of the program
  myHold = false; // false to run consumer thread

  if (pthread_create(&conThread, NULL, Consumerloop, NULL) == 0) {
    printf("CONSUMER THREAD CREATED\n");
    Producerloop();
    myHold = true;
    pthread_join(conThread, NULL); 
  } else {
    perror("Failed to create consumer thread\n");
  }
  printf("PROGRAM END\n");
  return 0;
}
