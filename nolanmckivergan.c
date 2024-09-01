//Nolan McKivergan
//12285017
//Added mutex to protect value of counter shared between two threads

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "common.h"
#include "common_threads.h"

int max;
volatile int counter = 0; // shared global variable
pthread_mutex_t mutex;//Shared global mutex

void *mythread(void *arg) {
    char *letter = arg;
    int i; // stack (private per thread) 
    printf("%s: begin [addr of i: %p,] [addr of counter: %p]\n", letter, &i,(unsigned int) &counter);
    for (i = 0; i < max * 1000; i++) {
        //Lock mutex
        pthread_mutex_lock(&mutex);

	    counter = counter + 1; // shared: only one
        
        //Unlock mutex
        pthread_mutex_unlock(&mutex);
    }
    printf("%s: done\n", letter);
    return NULL;
}
                                                                             
int main(int argc, char *argv[]) {                    
    if (argc != 2) {
	fprintf(stderr, "usage: main-first <loopcount>\n");
	exit(1);
    }
    max = atoi(argv[1]);

    pthread_t p1, p2;
    printf("main: begin [counter = %d] [%x]\n", counter, 
	   (unsigned int) &counter);

    //Initialize mutex
    pthread_mutex_init(&mutex, NULL);

    Pthread_create(&p1, NULL, mythread, "A"); 
    Pthread_create(&p2, NULL, mythread, "B");
    // join waits for the threads to finish
    Pthread_join(p1, NULL); 
    Pthread_join(p2, NULL); 

    //Destroy mutex after use
    pthread_mutex_destroy(&mutex);

    printf("main: done\n [counter: %d]\n [should: %d]\n", 
	   counter, max*2*1000);
    return 0;
}

