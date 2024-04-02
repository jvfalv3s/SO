/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h> 
#include <pthread.h>
#include "../LogFileManager/LogFileManager.h"
#include "./AutorizationReqManager.h"

/* Comment this line to don't show debug messages */
#define DEBUG

/* Initialization of the threads IDs */
pthread_t Sender_id, Receiver_id;

/**
 * Creates the Autorization Request Manager process.
 */
void AutReqMan() {
    /* Creates two threads, the sender and the receiver and logs their creation right after */
    pthread_create(&Sender_id, NULL, Sender, NULL);
    writeLog("THREAD SENDER CREATED");
    pthread_create(&Receiver_id, NULL, Receiver, NULL);
    writeLog("THREAD RECEIVER CREATED");

    //signal(SIGQUIT, endAutReqMan);

    /* Waits the threads to end */
    pthread_join(Sender_id, NULL);
    pthread_join(Receiver_id, NULL);

    exit(EXIT_SUCCESS);
}

/**
 * Sender Thread.
 */
void* Sender(void* arg) {
    (void) arg;
    #ifdef DEBUG
        printf("Thread Sender created.");
    #endif
    pthread_exit(NULL);
}

/**
 * Receiver Thread.
 */
void* Receiver(void* arg) {
    (void) arg;
    #ifdef DEBUG
        printf("Thread Receiver created.");
    #endif
    pthread_exit(NULL);
}

/**
 * Ends the Autorization Request Manager and his threads.
 * Not in final form.
 */
void endAutReqMan() {
    pthread_kill(Sender_id, SIGINT);
    pthread_kill(Sender_id, SIGINT);
    
    exit(EXIT_SUCCESS);
}

