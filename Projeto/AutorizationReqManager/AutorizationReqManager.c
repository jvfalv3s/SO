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
#include <stdbool.h>
#include "../LogFileManager/LogFileManager.h"
#include "./AutorizationReqManager.h"

/* Comment this line to don't show debug messages */
#define DEBUG

/* Paths to user and back pipe */
#define USER_PIPE_PATH "../tmp/FIFO/user_pipe"
#define BACK_PIPE_PATH "../tmp/FIFO/back_pipe"

/* Initialization */
pthread_t Sender_id, Receiver_id;  // Threads IDs
int user_pipe_fd, back_pipe_fd;    // User and back pipes file descriptors
bool userPipeCreated = false, backPipeCreated = false;  // User and back pipes creation status

/**
 * Creates the Autorization Request Manager process.
 */
void AutReqMan(pid_t SYS_PID) {
    /* Creates two threads, the sender and the receiver and logs their creation right after */
    pthread_create(&Sender_id, NULL, Sender, NULL);
    writeLog("THREAD SENDER CREATED");
    pthread_create(&Receiver_id, NULL, Receiver, NULL);
    writeLog("THREAD RECEIVER CREATED");

    /* Stays alert for sigquit signals */
    signal(SIGQUIT, endAutReqMan);

    /* Create USER_PIPE and BACK_PIPE */
    if(mkfifo(USER_PIPE_PATH, 0666) == -1) {
        error("CREATING USER PIPE");
        kill(SYS_PID, SIGQUIT);
        kill(0, SIGQUIT);
        endAutReqMan();
    }
    userPipeCreated = true;
    user_pipe_fd = open(USER_PIPE_PATH, O_RDONLY);
    if (user_pipe_fd == -1) {
        error("OPENING USER PIPE FOR READ");
        kill(SYS_PID, SIGQUIT);
        kill(0, SIGQUIT);
        endAutReqMan();
    }

    if(mkfifo(BACK_PIPE_PATH, 0666) == -1) {
        error("CREATING BACK PIPE");
        kill(SYS_PID, SIGQUIT);
        kill(0, SIGQUIT);
        endAutReqMan();
    }
    backPipeCreated = true;
    back_pipe_fd = open(BACK_PIPE_PATH, O_RDONLY);
    if (back_pipe_fd == -1) {
        error("OPENING BACK PIPE FOR READ");
        kill(SYS_PID, SIGQUIT);
        kill(0, SIGQUIT);
        endAutReqMan();
    }

    /* read example
    char message[100];
    read(user_pipe_fd, message, sizeof(message));
    */

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
 * Kills the Sender and Receiver threads.
 */
void killThreads() {
    pthread_kill(Sender_id, SIGINT);
    pthread_kill(Sender_id, SIGINT);
}

/**
 * Unlink all created pipes.
 */
void unlinkPipes() {
    if(userPipeCreated) unlink(USER_PIPE_PATH);
    if(backPipeCreated) unlink(BACK_PIPE_PATH);
}

/**
 * Ends the Autorization Request Manager and his threads.
 * Not in final form.
 */
void endAutReqMan() {
    killThreads();
    unlinkPipes();
    exit(EXIT_SUCCESS);
}

