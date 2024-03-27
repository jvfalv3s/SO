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

/* Comment this line to don't show debug mensages */
#define DEBUG

/**
 * Creates the Autorization Request Manager process.
 */
void AutReqMan(const char* logFileName) {
    /* Creates two threads, the sender and the receiver and logs their creation right after */
    pthread_t Sender_id, Receiver_id;
    pthread_create(&Sender_id, NULL, Sender, NULL);
    writeLog(logFileName, "THREAD SENDER CREATED");
    pthread_create(&Receiver_id, NULL, Receiver, NULL);
    writeLog(logFileName, "THREAD RECEIVER CREATED");

    //signal(SIGQUIT, endAutReqMan);

    /* Waits the threads to end */
    pthread_join(Sender_id, NULL);
    pthread_join(Receiver_id, NULL);

    exit(0);
}

/**
 * Sender Thread.
 */
void* Sender() {
    #ifdef DEBUG
        printf("Thread Sender created.");
    #endif
    return NULL;
}

/**
 * Receiver Thread.
 */
void* Receiver() {
    #ifdef DEBUG
        printf("Thread Receiver created.");
    #endif
    return NULL;
}

/**
 * Ends the Autorization Request Manager and his threads.
 * Not in final form.
 */
void endAutReqMan() {
    pthread_kill(Sender_id, SIGINT);
    pthread_kill(Sender_id, SIGINT);

    exit(0);
}

