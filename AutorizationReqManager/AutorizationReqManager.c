#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h> 
#include <pthread.h>

/* Comment this line to don't show debug mensages */
#define DEBUG

void* Sender() {
    #ifdef DEBUG
        printf("Thread Sender created.");
    #endif
    return NULL;
}

void* Receiver() {
    #ifdef DEBUG
        printf("Thread Receiver created.");
    #endif
    return NULL;
}

void AutReqMan() {
    pthread_t Sender_id, Receiver_id;
    pthread_create(&Sender_id, NULL, Sender, NULL);
    pthread_create(&Receiver_id, NULL, Receiver, NULL);
}

