// Mandatory Header Files Imports
#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
#include<pthread.h>
#include<time.h>
#include<unistd.h>


/**************************************************************************************************** 
 *  Code written to handle producer consumer problem                                                *    
 *  using semaphores.                                                                               *
 *  Here's how this works. We will assume that there can                                            *    
 *  be arbitrary number of producers as well as consumers.                                          *
 *  All of them are writing data to and reading data from                                           *
 *  the same shaerd buffer. The buffer is circular.                                                 *        
 *                                                                                                  *
 *  Roles:                                                                                          *
 *                                                                                                  *
 *  producer : places random charcters into the buffer of                                           *                
 *             size N.                                                                              *    
 *  consumer : reads the character from the buffer if there's                                       *
 *             data to read.                                                                        *    
 *                                                                                                  *
 *  solution:                                                                                       *
 *      1.) A semaphore mutexP is necessary for mutual exclusion                                    *
 *          in the producer's side because, multiple producer threads                               *            
 *          should not be able to write data to the shared buffer                                   *
 *          at once at the same position.                                                           *
 *                                                                                                  *
 *      2.) A semaphore mutexC is necessary for mutual exclusion                                    *
 *          in the consumer's side because, multiple consumer threads                               *
 *          should not be able to read the same buffer position.                                    *    
 *                                                                                                  *
 *      3.) A semaphore nrempty is necessary for producers to wait, be-                             *
 *          cause, buffer is circular, so previously written values should                          *
 *          not be over-written. The semaphore should be initialized to                             *
 *          positive integer N, where N is the size of the buffer.                                  *
 *                                                                                                  *
 *      4.) A semaphore named nrfull is necessary for consumers to wait, it                         *
 *          follows from logic similar to mentioned in point 3. This should be                      *
 *          initialized to 0, because initially there will be no content.                           *
 *                                                                                                  * 
 *      5.) The signal operation(V) for nrfull happens at the producer's side.                      *
 *          The signal operation(V) for nrempty happens at consumer's side.                         *
 *                                                                                                  *
 *       Usage:                                                                                     *
 *       Compile: gcc -o prod_cons.o prod_cons.c                                                    *
 *       Run: ./prod_cons.o -p <Number of producers> -c <Number of consumers>                       *
 ****************************************************************************************************/


// Size of shared buffer is 10 in this case
#define N 10

// A semaphore for waiting on full buffer.
sem_t nrfull;

// A semaphore for waiting on empty buffer.
sem_t nrempty;

// A semaphore for mutual exclusion(producers)
sem_t mutexP;

// A semaphore for mutual exclusion(consumers)
sem_t mutexC;


// Shared Buffer of size N;
char *buffer;
// Shared in and out index for circular buffer
int in=0, out=0;


/* This producer function continously produces
    a random charcter and tries to write it to 
    the shared buffer. If the shared buffer
    is full. It will stop writing.
*/
void * producer_func(void *arg){
    char message;
    while(1){
        message = 'A' + random()%26;
        sem_wait(&mutexP); // One producer at a time
        sem_wait(&nrempty); // Buffer should be empty for atleast 1 write
        buffer[in] = message; 
        printf("Producer %d wrote : %c\n", (int) arg, message);
        in = (in+1) % N;
        sem_post(&nrfull); // Atleast one value has been written to the buffer
        sem_post(&mutexP); // Allow other producers to come in
        sleep(1);          // A second's delay
    }
}

void * consumer_func(void *arg){
    char message;
    while(1){
        sem_wait(&mutexC); // One consumer at a time
        sem_wait(&nrfull); // Initially, nothing to consume nrfull:=0
        message = buffer[out];
        out = (out+1) % N;
        printf("Consumer %d consumed: %c \n", (int) arg, message);
        sem_post(&nrempty); // there is room for one more character in the buffer
        sem_post(&mutexC);  // Allow other consumers to come in
        sleep(1);           // A second's delay
    }
}


// main method
int main(int argc, char **argv){
    int opt;
    int nc, np;
    
    if(argc < 5){ 
        fprintf(stderr, "Usage: %s -p [number of producers] -c[number of consumers]\n ", argv[0]);
        exit(EXIT_FAILURE);
    }
    // parsing commandline options
    while((opt=getopt(argc, argv, "p:c:")) != -1){
        switch(opt){
            case 'p':
                np = atoi(optarg);
                break;
            case 'c':
                nc = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -p [number of producers] -c[number of consumers]\n ", argv[0]);
                exit(EXIT_FAILURE);
        }
    }// end parsing command line options

    printf("Producers: %d \t Consumers: %d\n", np, nc);

    int sret, tret;
    // Dedicate separate threads for producers and consumers.
    pthread_t producer_thread[np], consumer_thread[nc];
    int i;

    buffer = (char *) malloc(sizeof(char)*N); // size of buffer is N.
    if(buffer == NULL){
        fprintf(stderr, "Error: Failed to allocate memory\n");
        exit(EXIT_FAILURE);
     }// end if
    sret = sem_init(&nrfull, 0, 0); // initialize nrfull to zero
    if(sret){
        fprintf(stderr, "Error: Failed to initialize semaphore.\n");
        exit(EXIT_FAILURE);
    }// end if

    sret = sem_init(&nrempty, 0, N); // initialize nrempty to size of buffer
    if(sret){
        fprintf(stderr, "Error: Failed to initialize semaphore.\n");
        exit(EXIT_FAILURE);
    }// end if
    

    sret = sem_init(&mutexP, 0, 1); // initialize mutexP to 1.
    if(sret){
        fprintf(stderr, "Error: Failed to initialize semaphore.\n");
        exit(EXIT_FAILURE);
    }// end if

    
    sret = sem_init(&mutexC, 0, 1); // initialize mutexC to 1.
    if(sret){
        fprintf(stderr, "Error: Failed to initialize semaphore.\n");
        exit(EXIT_FAILURE);
    }// end if

 
    // Create different producer threads
    for(i = 0; i < np; i++){
        // Create a producer thread
        tret = pthread_create(&producer_thread[i], NULL, producer_func, (void *) i+1);
        if(tret){
            fprintf(stderr, "Error: Failed to start a new thread");
            exit(EXIT_FAILURE);
        }// end if
    }// end for
    
    // Create different consumer threads
    for(i = 0; i < nc; i++){
        // Create a consumer thread
        tret = pthread_create(&consumer_thread[i], NULL, consumer_func, (void *) i+1);
        if(tret){
            fprintf(stderr, "Error: Failed to start a new thread");
            exit(EXIT_FAILURE);
        }// end if
    }// end for
    
   
    for(i=0; i<np; i++)
        pthread_join(producer_thread[i], NULL);
    for(i=0; i<nc; i++)
        pthread_join(consumer_thread[i], NULL);
    return 0;
}



