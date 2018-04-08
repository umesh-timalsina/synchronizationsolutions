// Mandatory Header Files Import
#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

// A mutex lock for ticket operation in sequencers
pthread_mutex_t tlock; 

// a mutex lock for advance operation in EventCounts
pthread_mutex_t alock;


// Define a structure named sequencer(seq_t)
typedef struct sequencer{
    int val;
}seq_t;

void init_seq_t(seq_t *seq){
    seq->val = 0;
}// initialize the value to zero

// Issue ticket to one thread at a time
int ticket(seq_t *seq){
    pthread_mutex_lock(&tlock);
    return seq->val++;  
    pthread_mutex_unlock(&tlock);
}// end function ticket

/* Define Event Counts and Related Operations */

// Define a structure named eventcount(event_count_t)
typedef struct eventcount{
    int E;
}event_count_t;

void init_event_count_t(event_count_t *ec){
    ec-> E = 0;
}// initialize the value to zero

void advance(event_count_t *ec){
    pthread_mutex_lock(&alock);
    ec->E++;
    pthread_mutex_unlock(&alock);
}// Advance the event count

int read(event_count_t *ec){
    return ec->E;
}// end read

void await(event_count_t *ec, int v){

}// end await

// The main method
int main(){
    /* Initialize the mutex lock for ticket operation */
    int mret = pthread_mutex_init(&tlock, NULL);
    if(mret){
        printf("Locking ticket operation has failed. Terminating...\n");
        exit(EXIT_FAILURE);
    }// end if

    mret = pthread_mutex_lock(&alock, NULL);
    if(mret){
        printf("Locking advance operation has failed. Terminating...");
        exit(EXIT_FAILURE);
    }// end if

    seq_t Cticket;
    init_seq_t(&Cticket);
    int t = ticket(&Cticket);
    printf("Initialized a sequencer \n, ticket number : %d\n", t);

    return 0;
}// end main
