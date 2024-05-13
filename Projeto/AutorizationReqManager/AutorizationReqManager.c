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
#include "../LogFileManager/LogFileManager.h"
#include "./AutorizationReqManager.h"

/* Comment this line to don't show debug messages */
#define DEBUG

#define MAX_CHAR_COMMAND 30
/* Paths to user and back pipe */
#define USER_PIPE_PATH "user_pipe"
#define BACK_PIPE_PATH "back_pipe"
/* ftok arguments to create the message queue key */
#define VID_MQ_KEY_PATH "video_message_queue"
#define OTHER_MQ_KEY_PATH "other_message_queue"
#define MQ_KEY_ID 'a'


/* Message from message queue struct */
typedef struct vid_mq_message {
    long mgg_type;
    int data_to_reserve;
}vid_mq_message;

/* Message from message queue struct */
typedef struct other_mq_message {
    long mgg_type;
    char command[MAX_CHAR_COMMAND];
    int data_to_reserve;
}other_mq_message;

typedef struct queue {
    int mq_id;
    int queue_pos;
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
struct vid_mq_message vid_message;       // Message from message queue
struct other_mq_message other_message;   // Message from message queue
bool vid_queueCreated = false, other_queueCreated = false;

pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Creates the Autorization Request Manager process.
 */
void AutReqMan(int monitor_engine_pid , int MAX_QUEUE_POS) {
    SYS_PID = getppid();
    MON_EN_PID = monitor_engine_pid;
    
    /* Stays alert for sigquit signals */
    signal(SIGQUIT, endAutReqMan);

    /* Create USER_PIPE and BACK_PIPE */
    if(mkfifo(USER_PIPE_PATH, 0666) == -1) autReqError("CREATING USER PIPE");
    userPipeCreated = true;
    user_pipe_fd = open(USER_PIPE_PATH, O_RDONLY);
    if (user_pipe_fd == -1) autReqError("OPENING USER PIPE FOR READ");
    userPipeFDOpened = true;

    if(mkfifo(BACK_PIPE_PATH, 0666) == -1) autReqError("CREATING BACK PIPE");
    backPipeCreated = true;
    back_pipe_fd = open(BACK_PIPE_PATH, O_RDONLY);
    if (back_pipe_fd == -1) autReqError("OPENING BACK PIPE FOR READ");
    backPipeFDOpened = true;

    /* Creating VIDEO QUEUE and OTHER QUEUE */
    int vid_mq_key = ftok(VID_MQ_KEY_PATH, MQ_KEY_ID);
    if(vid_mq_key == -1) error("Creating message queue key");
    vid_queue.mq_id = msgget(vid_mq_key, IPC_CREAT | 0666);
    if(vid_queue.mq_id == -1) error("Getting message queue id");
    vid_queueCreated = true;
    vid_queue.queue_pos = 0;
    vid_queue.max_queue_pos = MAX_QUEUE_POS;

    int other_mq_key = ftok(OTHER_MQ_KEY_PATH, MQ_KEY_ID);
    if(other_mq_key == -1) error("Creating message queue key");
    other_queue.mq_id = msgget(other_mq_key, IPC_CREAT | 0666);
    if(other_queue.mq_id == -1) error("Getting message queue id");
    other_queueCreated = true;
    other_queue.queue_pos = 0;
    other_queue.max_queue_pos = MAX_QUEUE_POS;
    
    /* Creates two threads, the sender and the receiver and logs their creation right after */
    pthread_create(&Sender_id, NULL, Sender, NULL);
    SenderCreated = true;
    writeLog("THREAD SENDER CREATED");
    pthread_create(&Receiver_id, NULL, Receiver, NULL);
    ReceiverCreated = true;
    writeLog("THREAD RECEIVER CREATED");

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

