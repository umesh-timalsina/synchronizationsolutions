// Mandatory Header Files Import
#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
#include<unistd.h>
#include<pthread.h>
#include<fcntl.h>

#define N 10 // Number of threads

void *thread_func(void *);
/****************************************************************************************************
    
    There are five threads, with thread ids 1, 2, 3, 4, 5 respectively. They are allowed to access
    a file descriptor simultaneously if and only if the PID of the entering threads sum up to a 
    value that is less than or equal to nine. Each thread will just read the file from top to bottom
    and rewind the file.
*****************************************************************************************************/


int fd; // The shared file descriptor

int sum=0; // The sum of all entering threadIDs
int waiting = 0;    // The number of processes waiting
sem_t fd_mutex;     // A semaphore for waiting on the File descriptor
sem_t sum_mutex;    // A semaphore for altering the value of sum.
char buf[500];      // A buffer to store read content
void * thread_func(void *arg){
    while(1){
        /* This part for getting access to the file */
        sem_wait(&sum_mutex);       // Wait for sum_mutex
        while((sum+(int)arg) > 10){
            waiting++; // increase the number of wating processes
            printf("Thread %d enters waiting queue\n", (int)arg);
            sem_post(&sum_mutex);
            sem_wait(&fd_mutex);
            sem_wait(&sum_mutex);
        }
        sum += (int) arg;   // Increment the value of sum
        sem_post(&sum_mutex);

        /* File Access Code goes here */
        printf("Thread %d can access the file now and is..\n", (int)arg);   // Get Regular Access to the File
        read(fd, buf, 50);
        printf("%s\n", buf);
        int i;
        sem_wait(&sum_mutex);       // Wait till we decrease the value
        sum -= (int) arg;
        for(i = 0; i < waiting; i++){
            sem_post(&fd_mutex);
        }
        waiting=0;
        sem_post(&sum_mutex);
        sleep(1);       // A second's delay
    }// end while
}// the thread function



int main(int argc, char **argv){
    
    int sret, tret, i;     
    
    pthread_t threads[N];
    
    sret = sem_init(&fd_mutex, 0, 0);   // Initialize the semaphore to one
    if(sret){
        printf("Failed to initialize semaphore.. Terminating...");
        exit(EXIT_FAILURE);
    }// end if
    
    sret = sem_init(&sum_mutex, 0, 1); // Initialize the semaphore to one
    if(sret){
        printf("Failed to initialize semaphore.. Terminating...");
        exit(EXIT_FAILURE);
    }// end if

    // Open a file named file.txt
    fd = open("file.txt", O_RDONLY);  
    
    for(i = N; i > 0; i--){
        tret = pthread_create(&threads[i], NULL, thread_func, (void *)i);
        if(tret){
            printf("Failed to create threads, quiting...");
            exit(EXIT_FAILURE);
        }
    }// end for

    for(i = 0; i < N; i++){
        pthread_join(threads[i], NULL);
    }
    return 0;
}// end function main.
