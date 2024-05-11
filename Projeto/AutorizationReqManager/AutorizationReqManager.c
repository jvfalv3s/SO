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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h> 
#include <pthread.h>
#include <stdbool.h>
#include <sys/select.h>
#include "../LogFileManager/LogFileManager.h"
#include "./AutorizationReqManager.h"

/* Comment this line to don't show debug messages */
#define DEBUG

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
}message;

typedef struct queue {
    struct message* messages;
    int read_pos;
    int write_pos;
    int max_queue_pos;
}queue;

/* Initialization */
pid_t SYS_PID;  // Parent (System Manager) PID
pid_t MON_EN_PID;
pthread_t Sender_id, Receiver_id;  // Threads IDs
int user_pipe_fd, back_pipe_fd;    // User and back pipes file descriptors
bool SenderCreated = false, ReceiverCreated = false;      // Sender and Receiver threads creation status
bool userPipeCreated = false, backPipeCreated = false;    // User and back pipes creation status
bool userPipeFDOpened = false, backPipeFDOpened = false;  // User and back pipes file descriptors open status

struct queue vid_queue;                  
struct queue other_queue;                
bool vid_queueCreated = false, other_queueCreated = false;

pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Creates the Autorization Request Manager process.
 */
void AutReqMan(pid_t monitor_engine_pid, int MAX_QUEUE_POS) {
    SYS_PID = getppid();
    MON_EN_PID = monitor_engine_pid;
    
    /* Stays alert for sigquit signals */
    signal(SIGQUIT, endAutReqMan);

    /* Creating USER_PIPE and BACK_PIPE */
    if(mkfifo(USER_PIPE_PATH, 0666) == -1) autReqError("CREATING USER PIPE");
    userPipeCreated = true;
    if(mkfifo(BACK_PIPE_PATH, 0666) == -1) autReqError("CREATING BACK PIPE");
    backPipeCreated = true;

    /* Creating VIDEO QUEUE and OTHER QUEUE */
    vid_queue.messages = (struct message*) malloc(MAX_QUEUE_POS * sizeof(struct message));
    vid_queueCreated = true;
    vid_queue.read_pos = 0;
    vid_queue.write_pos = 0;
    vid_queue.max_queue_pos = MAX_QUEUE_POS;

    other_queue.messages = (struct message*) malloc(MAX_QUEUE_POS * sizeof(struct message));
    other_queueCreated = true;
    other_queue.read_pos = 0;
    other_queue.write_pos = 0;~
    other_queue.max_queue_pos = MAX_QUEUE_POS;
    
    /* Creates two threads, the sender and the receiver and logs their creation right after */
    pthread_create(&Sender_id, NULL, Sender, NULL);
    SenderCreated = true;
    writeLog("THREAD SENDER CREATED");
    pthread_create(&Receiver_id, NULL, Receiver, NULL);
    ReceiverCreated = true;
    writeLog("THREAD RECEIVER CREATED");

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

    /* Opening USER_PIPE and BACK_PIPE */
    user_pipe_fd = open(USER_PIPE_PATH, O_RDONLY);
    if (user_pipe_fd == -1) autReqError("OPENING USER PIPE FOR READ");
    userPipeFDOpened = true;
    back_pipe_fd = open(BACK_PIPE_PATH, O_RDONLY);
    if (back_pipe_fd == -1) autReqError("OPENING BACK PIPE FOR READ");
    backPipeFDOpened = true;

    fd_set read_set;
    struct queue* queue_ptr;
    int nready;
    int nread;
    char buf[MAX_CHAR_COMMAND];
    char* arg1;
    char* arg2;
    char* arg3;
    char* log_message;

    /* Clear the descriptor set */
    FD_ZERO(&read_set);

    /* Gets the max fd */
    int maxfdp = user_pipe_fd;
    if(back_pipe_fd > maxfdp) maxfdp = back_pipe_fd;
    maxfdp++;

    while(true) {
        /* Sets all pipes in the read set */
        FD_SET(user_pipe_fd, &read_set);
        FD_SET(back_pipe_fd, &read_set);

        /* Selects the ready descriptor (the pipe with information in) */
        nready = select(maxfdp, &read_set, NULL, NULL, NULL);

        if(nready > 0) {
            if(FD_ISSET(user_pipe_fd, &read_set)) {
                nread = read(user_pipe_fd, buf, sizeof(buf));
                if(nread == -1) autReqError("READING FROM USER PIPE");
                buf[nread] = '\0';
                #ifdef DEBUG
                    printf("buf = %s\n", buf);
                #endif
                div_buf_info(buf, arg1, arg2, arg3);
                if(strcmp(arg2, "VIDEO") == 0) queue_ptr = &vid_queue;
                else queue_ptr = &other_queue;

                pthread_mutex_lock(&mutex);
                write_in_queue(queue_ptr, arg1, arg2, arg3);
                pthread_mutex_unlock(&mutex);
            }

            if(FD_ISSET(back_pipe_fd, &read_set)) {
                nread = read(back_pipe_fd, buf, sizeof(buf));
                if(nread == -1) autReqError("READING FROM BACK PIPE");
                buf[nread] = '\0';
                #ifdef DEBUG
                    printf("buf = %s\n", buf);
                #endif
                div_buf_info(buf, arg1, arg2, arg3);

                pthread_mutex_lock(&mutex);
                write_in_queue(&other_queue, arg1, arg2, arg3);
                pthread_mutex_unlock(&mutex);
            }
        }
    }

    pthread_exit(NULL);
}

/**
 * Divides buffer info from pipes to the given char pointers.
*/
void div_buf_info(char* buf, char* arg1, char* arg2, char* arg3) {
    arg1 = strtok(buf, "#");
    arg2 = strtok(NULL, "#");
    arg3 = strtok(NULL, "#");
}

/**
 * Writes in a queue the new message (data, can be null).
 */
void write_in_queue(struct queue* queue_ptr, char* ID, char* command, char* data) {
    char* log_message;
    if(((queue_ptr->write_pos+1) % queue_ptr->max_queue_pos) == queue_ptr->read_pos) {
        if(sprintf(log_message, "RECEIVER: WARNING -> QUEUE IS FULL (ID = %s)", ID) < 0) autReqError("CREATING WARNING LOG MESSAGE");
        writeLog(log_message);
    }
    else {
        queue_ptr->messages[queue_ptr->write_pos].id = atoi(ID);
        queue_ptr->messages[queue_ptr->write_pos].command = command;
        if(data != NULL) queue_ptr->messages[queue_ptr->write_pos].data_to_reserve = atoi(data);
        queue_ptr->write_pos = (queue_ptr->write_pos+1) % queue_ptr->max_queue_pos;
    }
}

/**
 * Kills System all processes sending SIGQUIT to them.
 */
void killSys() {
    kill(0, SIGQUIT);
    kill(SYS_PID, SIGQUIT);
}

/**
 * Kills the Sender and Receiver threads.
 */
void killThreads() {
    if(SenderCreated) pthread_kill(Sender_id, SIGINT);
    if(ReceiverCreated) pthread_kill(Receiver_id, SIGINT);
}

/**
 * Unlink all created pipes and closes all file descriptors.
 */
void unlinkPipes() {
    if(userPipeCreated) unlink(USER_PIPE_PATH);
    if(backPipeCreated) unlink(BACK_PIPE_PATH);
    if(userPipeFDOpened) close(user_pipe_fd);
    if(backPipeFDOpened) close(back_pipe_fd);
}

/**
 * Reports error ocurred printing it in screen and into log file and ends all processes.
 */
void autReqError(char* error_message) {
    error(error_message);
    killSys();
    endAutReqMan();
}

/**
 * Ends the Autorization Request Manager and his threads.
 */
void endAutReqMan() {
    killThreads();
    unlinkPipes();
    exit(EXIT_SUCCESS);
}

