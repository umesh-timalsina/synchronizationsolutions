//  Header files Import
#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>
#include<unistd.h>
/** 
    An implementation of mutual exclusion between 
    two threads using semaphores.

    Program enters the main method, creates two 
    threads, some shared resource (in this case 
    an integer variable) is used to access the
    shared data safely. The major idea behind
    here is to show how mutual exclusion can be
    achieved using the wait(P) and signal(V) 
    operations on the semaphore.
*/


// A single semaphore is declared here...
sem_t semaphore;

// This is the single shared variable
int shared = 0;

// The function call in the first thread
void *shared_func1(void *ptr){
    int i = 0;
    sem_wait(&semaphore); // proper error handling necessary
    fprintf(stdout, "Incrementing value of shared\n");
    for(i = 0; i<10; i++){
        printf("Value of shared = %d\n", ++shared);
        sleep(1);
    }// end for
    sem_post(&semaphore); // proper error handling necessary
}// end shared_func1

// the function call in the second thread
void *shared_func2(void *ptr){
    int i = 0;
    sem_wait(&semaphore); // proper error handling necessary
    fprintf(stdout, "Decrementing value of shared\n");
    for(i = 0; i<10; i++){
        printf("Value of shared = %d\n", --shared);
        sleep(1);
    }// end for
    sem_post(&semaphore); // proper error handling necessary
}// end shared_func2

int main(){
    
    pthread_t thread1, thread2; 
    int ret1, ret2, sem_ret;
    
    // Initialize the semaphore
    // The semaphore being used is
    // semaphore(global), it cannot 
    // be shared between forked processes(0)
    // and it has an initial value of zero
    sem_init(&semaphore, 0, 1);
    if(sem_ret){
        fprintf(stderr, "Failed to initialize semaphore");
        exit(EXIT_FAILURE);
    }// end if

    // Create a new thread 
    ret1 = pthread_create(&thread1, NULL, 
                    shared_func1, NULL);
    if(ret1){
        fprintf(stderr, "Failed to create new thread");
        exit(EXIT_FAILURE);
    }

    // Create another thread
    ret2 = pthread_create(&thread2, NULL,
                    shared_func2, NULL);

    if(ret2){
        fprintf(stderr, "Failed to create new thread");
        exit(EXIT_FAILURE);
    }
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("The value of shared is %d \n.", shared);
    return 0;
}// end main
