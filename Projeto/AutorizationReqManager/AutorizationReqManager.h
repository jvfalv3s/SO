/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#ifndef AUTORIZATIONREQMANAGER_H
#define AUTORIZATIONREQMANAGER_H

#define BUF_SIZE 100
#define MAX_CHAR_COMMAND 30
/* Paths to user and back pipe */
#define USER_PIPE_PATH "user_pipe"
#define BACK_PIPE_PATH "back_pipe"

/* Message from message queue struct */
typedef struct message {
    int id;
    char command[MAX_CHAR_COMMAND];
    int data_to_reserve;
    time_t request_time;
}message;

typedef struct queue {
    struct message* messages;
    int read_pos;
    int write_pos;
    int max_queue_pos;
    int n_empty;
}queue;

/**
 * Creates the Autorization Request Manager process.
 */
void AutReqMan();

/**
 * Sender Thread.
 */
void* Sender();

/**
 * Receiver Thread.
 */
void* Receiver();

/**
 * Kills System all processes sending SIGQUIT to them.
 */
void killSys();

/**
 * Kills the Sender and Receiver threads.
 */
void killThreads();

/**
 * Unlink all created pipes and closes all file descriptors.
 */
void unlinkPipes();

/**
 * Reports error ocurred printing it in screen and into log file and ends all processes.
 */
void autReqError(char* error_message);

/**
 * Ends the Autorization Request Manager and his threads.
 */
void endAutReqMan();

#endif