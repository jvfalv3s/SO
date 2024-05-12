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
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/select.h>
#include "../LogFileManager/LogFileManager.h"
#include "./AutorizationReqManager.h"
#include "./AuthorizationEngine.h"
#include "ShmData.h"

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

/* Initialization */
struct shm_struct* shm_ptr;
pid_t SYS_PID;  // Parent (System Manager) PID
pid_t MON_EN_PID;
int MAX_QUEUE_POS, AUTH_SERVERS_MAX, AUTH_PROC_TIME, MAX_VIDEO_WAIT, MAX_OTHERS_WAIT;
pthread_t Sender_id, Receiver_id;  // Threads IDs
int user_pipe_fd, back_pipe_fd;    // User and back pipes file descriptors
bool SenderCreated = false, ReceiverCreated = false;      // Sender and Receiver threads creation status
bool userPipeCreated = false, backPipeCreated = false;    // User and back pipes creation status
bool userPipeFDOpened = false, backPipeFDOpened = false;  // User and back pipes file descriptors open status

struct auth_eng* auth_engs;

struct queue vid_queue;                  
struct queue other_queue;                
bool vid_queueCreated = false, other_queueCreated = false;

pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Creates the Autorization Request Manager process.
 */
void AutReqMan(struct shm_struct* shmPtr, pid_t monitor_engine_pid, int max_queue_pos, int auth_servers_max, int auth_proc_time, int max_video_wait, int max_other_wait) {
    shm_ptr = shmPtr;
    SYS_PID = getppid();
    MON_EN_PID = monitor_engine_pid;
    MAX_QUEUE_POS = max_queue_pos;
    AUTH_SERVERS_MAX = auth_servers_max;
    AUTH_PROC_TIME = auth_proc_time;
    MAX_VIDEO_WAIT = max_video_wait;
    MAX_OTHERS_WAIT = max_other_wait;
    
    /* Stays alert for sigquit signals */
    signal(SIGQUIT, endAutReqMan);

    /* Creating USER_PIPE and BACK_PIPE */
    if(mkfifo(USER_PIPE_PATH, 0666) == -1) autReqError("CREATING USER PIPE");
    userPipeCreated = true;
    if(mkfifo(BACK_PIPE_PATH, 0666) == -1) autReqError("CREATING BACK PIPE");
    backPipeCreated = true;

    /* Creating VIDEO QUEUE and OTHER QUEUE */
    create_queues();

    /* Creating all the authorizations engines */
    shm_ptr->auth_engs = (struct auth_eng*) malloc((AUTH_SERVERS_MAX+1) * sizeof(struct auth_eng));
    for(int i = 0; i < AUTH_SERVERS_MAX; i++) create_auth_eng(i, MON_EN_PID);
    
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


/***********************************************************************
 *                                                                     *
 *                               THREADS                               *
 *                                                                     *
 ***********************************************************************/

/**
 * Sender Thread.
 */
void* Sender(void* arg) {
    (void) arg;
    #ifdef DEBUG
        printf("Thread Sender created.");
    #endif

    struct message request;
    char* log_message;
    int auth_eng_num;
    while(true) {
        pthread_mutex_lock(&mutex);

        while((vid_queue.n_empty == vid_queue.max_queue_pos) && (other_queue.n_empty == other_queue.max_queue_pos)) {
            pthread_cond_wait(&cv, &mutex);
        }

        update_queues();

        check_auth_busy();
        if(any_queue_is(1.00)) create_auth_eng(AUTH_SERVERS_MAX, MON_EN_PID);
        else if(any_queue_is(0.50)) remove_auth_eng(AUTH_SERVERS_MAX);

        auth_eng_num = get_auth_eng_num();
        if(auth_eng_num != -1) {
            if(vid_queue.n_empty != vid_queue.max_queue_pos) {
                get_from_queue(&vid_queue, &request);
                send_req_to(auth_eng_num, request);

                if(sprintf(log_message, "SENDER: VIDEO AUTHORIZATION REQUEST (ID = %d) SENT FOR PROCESSING ON AUTHORIZATION_ENGINE %d", request.id, auth_eng_num) < 0) {
                    error("CREATING SENDER VIDEO AUTHORIZATION REQUEST LOG MESSAGE");
                }
                writeLog(log_message);
            }
            else if(other_queue.n_empty != other_queue.max_queue_pos) {
                get_from_queue(&other_queue, &request);
                send_req_to(auth_eng_num, request);

                if(strcmp(request.command, "SOCIAL") == 0) {
                    if(sprintf(log_message, "SENDER: SOCIAL AUTHORIZATION REQUEST (ID = %d) SENT FOR PROCESSING ON AUTHORIZATION_ENGINE %d", request.id, auth_eng_num+1) < 0) {
                        error("CREATING SENDER SOCIAL AUTHORIZATION REQUEST LOG MESSAGE");
                    }
                    writeLog(log_message);
                }
                else if(strcmp(request.command, "MUSIC") == 0) {
                    if(sprintf(log_message, "SENDER: MUSIC AUTHORIZATION REQUEST (ID = %d) SENT FOR PROCESSING ON AUTHORIZATION_ENGINE %d", request.id, auth_eng_num+1) < 0) {
                        error("CREATING SENDER MUSIC AUTHORIZATION REQUEST LOG MESSAGE");
                    }
                    writeLog(log_message);
                }
            }
        }
        pthread_mutex_unlock(&mutex);
    }
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


/***********************************************************************
 *                                                                     *
 *                           HELP FUNCTIONS                            *
 *                                                                     *
 ***********************************************************************/

/**
 * Creates the 2 queues, vid_queue and other_queue.
 */
void create_queues() {
    struct queue* queue_ptr;
    for(int i = 0; i < 2; i++) {
        if(i == 0) queue_ptr = &vid_queue;
        else queue_ptr = &other_queue;

        queue_ptr.messages = (struct message*) malloc(MAX_QUEUE_POS * sizeof(struct message));
        queue_ptr.read_pos = 0;
        queue_ptr.write_pos = 0;
        queue_ptr.max_queue_pos = MAX_QUEUE_POS;
        queue_ptr.n_empty = MAX_QUEUE_POS;

        if(i == 0)  vid_queueCreated = true;
        else other_queueCreated = true;
    }
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
        queue_ptr->messages[queue_ptr->write_pos].request_time = time(0);
        queue_ptr->write_pos = (queue_ptr->write_pos+1) % queue_ptr->max_queue_pos;
    }
}

/**
 * Gets a message from the queue indicated by the pointer and return the data in the struct indicated by the pointer.
 */
void get_from_queue(struct queue* queue_ptr, struct message* request) {
    request->id = queue_ptr->messages[queue_ptr->read_pos].id;
    strcpy(request->command, queue_ptr->messages[queue_ptr->read_pos].command);
    request->data_to_reserve = queue_ptr->messages[queue_ptr->read_pos].data_to_reserve;
    request->request_time = queue_ptr->messages[queue_ptr->read_pos].request_time;
}

/**
 * Updates the queues eliminating expired requests.
 */
void update_queues() {
    struct queue* queue_ptr;
    struct message request;
    int max_time;
    int checked;
    double dif;
    char* log_message;

    for(int i = 0; i < 2; i++) {
        if(i == 0) {
            queue_ptr = &vid_queue;
            max_time = MAX_VIDEO_WAIT;
        }
        else {
            queue_ptr = &other_queue;
            max_time = MAX_OTHERS_WAIT;
        }

        checked = 0;
        while((queue_ptr->max_queue_pos - queue_ptr->n_empty - checked) > 0) {
            dif = difftime(queue_ptr->messages[queue_ptr->read_pos].request_time, time(0));
            if(dif > max_time * 1000) {
                get_from_queue(queue_ptr, &request);
                if(sprintf(log_message, "SENDER: WARNING -> DELITING EXPIRED REQUEST (ID = %s)", request.id) < 0) autReqError("CREATING WARNING LOG MESSAGE");
                writeLog(log_message);
            }
            queue_ptr->read_pos = (queue_ptr->read_pos + 1) % queue_ptr->max_queue_pos;
            checked++;
        }
    }
}

/**
 * Check if any of the two internal queues are at 'percentage' of their total size.
 */
bool any_queue_is(double percentage){
    if((vid_queue.n_empty/vid_queue.max_queue_pos) <= percentage) return true;
    else return false;
}


/***********************************************************************
 *                                                                     *
 *                   RESOURCES MANAGEMENT FUNCTIONS                    *
 *                                                                     *
 ***********************************************************************/

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

