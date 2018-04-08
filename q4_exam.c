// Mandatory Header Files import
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

sem_t mutexone, mutextwo, wrt, z;
char message;
int nreaders=0;

void * reader_func(void * arg){ 
    while(1){
        char msg_read;
        sem_wait(&mutexone);
        nreaders++; 
        if(nreaders==1) 
            sem_wait(&wrt);
        sem_post(&mutexone);
        msg_read = message;
        printf("Read the following message: %c\n", msg_read);
        sem_wait(&mutextwo);
        nreaders--;
        if(nreaders==0)
            sem_post(&wrt);
        sem_post(&mutextwo);
        sem_post(&z);

        sleep(1); // Set a second's interval
    }
}// the reader function

void * writer_func(void * arg){
    while(1){
        sem_wait(&z); // Initially wait
        sem_wait(&wrt);
        message = 65 + random()%26;// the write operation
        printf("Wrote the following message: %c\n", message);
        sem_post(&wrt);
        sleep(1);
    }
}// the writer function

int main(int argc, char **argv){
    int sret, tret;

    // Initialize the semaphores to one
    sret = sem_init(&mutexone, 0, 1);
    sret = sem_init(&mutextwo, 0, 1);
    sret = sem_init(&wrt,      0, 1);
    sret = sem_init(&z,        0, 0); // z to zero
    if(sret){
        printf("Initialization of semaphores failed..Terminating");
        exit(EXIT_FAILURE);
    }
    // create two threads
    pthread_t reader1, reader2, writer;

    tret = pthread_create(&writer ,  NULL, writer_func, NULL);
    tret = pthread_create(&reader1,  NULL, reader_func, NULL);
    tret = pthread_create(&reader2,  NULL, reader_func, NULL);
    if(tret){
        printf("Failed to create threads... Terminating");
        exit(EXIT_FAILURE);
    }
    pthread_join(reader1, NULL);
    pthread_join(reader2, NULL);
    pthread_join(writer, NULL);
}
