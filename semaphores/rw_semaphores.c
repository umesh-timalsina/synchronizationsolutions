#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<semaphore.h>
#include<pthread.h>

/********************************************************************************************************************
 * An implementation of reader writer solution in threads using semaphores in C.                                    |
 *                                                                                                                  |
 * The problem: Let's assume there are some shared resources like a fileDescriptor.                                 |
 * The file descriptor may be read my multiple parties, however, we have to make sure that                          |
 * the writers to the FD can do so in a singleton fashion. Secondly, when a reader is reading                       |
 * the file descriptor, a writer should not be simultanously allwoed to write to it.                                |
 *                                                                                                                  |
 * Solution Using Semaphores:                                                                                       |
 *  The solution is straight forward. We need following shared variables and semaphores                             |
 *      -- var wmutex, rmutex(semaphores) :=1,1;                                                                    |
 *      -- nreaders: integer := 0                                                                                   |
 * 1.) In the reader threads side only one reader should enter the critical section: P(rmutex).                     |
 *     After entering, it must check wheter it is the first reader entering the                                     |        
 *     critical section in recent time(nreaders == 0). Then it must perform a P(wmutex)                             |
 *     indicating that no writers will be allowed now. Also, increase the readers count(nreaders++).                |
 *                                                                                                                  |
 * 2.) After that, the reader should perform a V(rmutex), so that others readers can come in.                       |
 *                                                                                                                  |
 * 3.) The readers read the critical section shared variables.                                                      |
 *                                                                                                                  |
 * 4.) After performing read operations, each reader must atomically reduce number of readers                       |
 *     (nreaders--) and exit the critical section P(rmutex). Before exiting the critical section                    |
 *     each reader must check whether the number of readers has reduced to zero. If that is the                     |
 *     case then, perform V(wmutex) so that writers can now come in and access the critical section.                |
 *     i. e. perform a write operation to the critical section.                                                     |
 *                                                                                                                  |
 * The following code implements this soltion using standard POSIX threads. Although, threads are different         |
 * that OS procesess, we can draw many analogical similarities between them.                                        |
 *                                                                                                                  |   
 *  Usage:                                                                                                          |
 *      Compile: gcc -o rw_semaphores.c rw_semaphores.o                                                             |
 *      Run:     ./rw_semaphores.o -r <Number of readers> -w <Number of writers>                                    |
 *                                                                                                                  |
 *                                                                                                                  |
 ********************************************************************************************************************/

const char *authors[5] = {"George RR Maritn", "Adolf Hitler", 
                           "Dennis Ritchie", "Andy Wang", 
                           "Karl Marx"                        };
const char *books[5] = {"A Song of Ice and Fire", "Mine Kamp", 
                        "The C Programming Language",  
                        "Component Oriented Programming",                        
                        "Communist Manifesto"};
/*
 * Shared section of the program
 */

typedef struct book{
    char *title;
    char *author; 
}Book;         

Book book1;  // this is the shared section
int nreaders = 0; // this is also shared

// Two semaphores for mutual exclusion in reader and writers
sem_t rmutex, wmutex;

// The function to be called by reader threads
void * reader_func(void *arg){
    while(1){
        sem_wait(&rmutex);  // Readers enter one at a time
        if(nreaders == 0){
        sem_wait(&wmutex); // The first reader to enter the game
        }
        nreaders++; // Increment number of readers
        sem_post(&rmutex); //Allow other readers to come in as well
        printf("Reader : %d, Read %s by %s \n", (int) arg, book1.title, book1.author);
        sem_wait(&rmutex); // Reading done, time to exit one at a time
        nreaders--;
        if(nreaders == 0)           // looks like nobody is here to read, 
            sem_post(&wmutex);      //allow writers to come in
        sem_post(&rmutex);     // Allow other readers to exit
        sleep(1);           // A delay of 1 seconds
    }
}// end function reader_func

// The function to be called by writer threads

void * writer_func(void *arg){
    int j;
    while(1){
        sem_wait(&wmutex);      // wait for other writers to finish
        j = random()%5;
        book1.title = books[j];
        book1.author = authors[j];
        printf("Writer: %d, Wrote %s by %s\n", (int) arg, book1.title,
                book1.author);
        sem_post(&wmutex);
        sleep(1); // A one second delay
    }
}//end function writer_func


int main(int argc, char **argv){
    int opt;
    int nr, nw;
    book1.title = "Harry Porter";
    book1.author = "JK ROWLING";
    if(argc < 2){ 
        fprintf(stderr, "Usage: %s -r [number of readers] -w[number of writers]\n ", argv[0]);
        exit(EXIT_FAILURE);
    }
    // parsing commandline options
    while((opt=getopt(argc, argv, "r:w:")) != -1){
        switch(opt){
            case 'r':
                nr = atoi(optarg);
                break;
            case 'w':
                nw = atoi(optarg);
                break;
            case '?':
            default:
                fprintf(stderr, "Usage: %s -p [number of producers] -c[number of consumers]\n ", argv[0]);
                exit(EXIT_FAILURE);
        }
    }// end parsing command line options

    printf("Readers: %d \t Writers: %d\n", nr, nw);

    int sret, tret, i; 
    pthread_t readers[nr], writers[nw];
    
    // initialize the rmutex to 1
    sret = sem_init(&rmutex, 0, 1);
    if(sret){
        fprintf(stderr, "Error: Failed to initialize semaphore");
        exit(EXIT_FAILURE);
    }// end if
    
    sret = sem_init(&wmutex, 0, 1);
    if(sret){
        fprintf(stderr, "Error: Failed to initialize semaphore");
        exit(EXIT_FAILURE);
    }// end if
    
    // Create reader threads
    for(i = 0; i < nr; i++){
        tret = pthread_create(&readers[i], NULL, reader_func, (void *) i+1);
        if(tret){
            fprintf(stderr, "Error: Failed to create thread");
            exit(EXIT_FAILURE);
        }// end if
    }// end for

    // Create writer threads
    for(i = 0; i < nw; i++){
        tret = pthread_create(&writers[i], NULL, writer_func, (void *) i+1);
        if(tret){
            fprintf(stderr, "Error: Failed to create thread");
            exit(EXIT_FAILURE);
        }// end if
    }// end for
    for(i=0; i<nr; i++)
        pthread_join(readers[i], NULL);
    for(i=0; i<nw; i++)
        pthread_join(writers[i], NULL);
    return 0;

}// end main

