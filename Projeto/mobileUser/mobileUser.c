/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "mobileUser.h"
#include <signal.h>
#include <ctype.h>
#include <sys/time.h>
#include <semaphore.h>
#include <stdbool.h>

/* Comment this line to don't show debug messages */
#define DEBUG

/* Max characters a command can have */
#define MAX_CHAR_COMMAND_AMMOUNT 20
/* Max characters a message can have */
#define MAX_CHAR_MESSAGE_AMMOUNT 100
/* ftok arguments to create the message queue key */
#define MQ_KEY_PATH "/message_queue"
#define MQ_KEY_ID 'a'
/* Path of the USER PIPE */
#define USER_PIPE_PATH "../tmp/FIFO/user_pipe"
/* Path to USER PIPE NAMED MUTEX SEMAPHORE */
#define USER_PIPE_MUTEX_PATH "../tmp/NAMED_SEMS/user_pipe_mutex_sem"
/* Path to MESSAGE QUEUE NAMED SEMAPHORE directory */
#define MQ_NAMED_SEMAPHORE_DIR_PATH "../tmp/NAMED_SEMS/"

/* Message from message queue struct */
typedef struct mq_message {
    long mgg_type;
    char msg_text [MAX_CHAR_MESSAGE_AMMOUNT];
};

/* Initializing some usefull variables */
pid_t MOBILE_USER_ID;                    // ID of the mobile user (PID)
char command[MAX_CHAR_COMMAND_AMMOUNT];
struct mq_message message;               // Message from message queue
int user_pipe_fd;                        // User pipe file descriptor
sem_t *user_pipe_mutex, *mq_named_sem;   // Named semaphores
char* mq_named_sem_path;                 // Path to message queue named semaphore
int mq_id;                               // Message queue id

/* Status variables */
bool userPipeFdOpened = false;
bool userPipeMutexOpened = false;
bool mqNamedSemCreated = false;

/**
 * Main function.
 */
int main(int argc, char **argv) {
    /* Error in case wrong usage of the file */
    if (argc != 7) {
        fprintf(stderr, "Use: %s <Initial_Plafond> <Max_Autorizations_Requests> <VIDEO_Interval> <MUSIC_Interval> <SOCIAL_Interval> <Data_to_Reserve>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* SIGINT handling */
    signal(SIGINT, handle_sigint);

    /* Getting mobile user ID (PID of the mobile user process) */
    MOBILE_USER_ID = getpid();

    /* CLI arguments extraction and verification */
    int initial_plafond = atoi(argv[1]);
    if(initial_plafond <= 0) {
        perror("Initial Plafond must be > 0");
        exit(EXIT_FAILURE);
    }
    #ifdef DEBUG
        printf("Initial Plafond = %d\n", initial_plafond);
    #endif

    int max_autorizations_requests = atoi(argv[2]);
    if(max_autorizations_requests <= 0) {
        perror("Max Autorizations Requests must be > 0");
        exit(EXIT_FAILURE);
    }
    #ifdef DEBUG
        printf("Max Autorizations Requests = %d\n", max_autorizations_requests);
    #endif

    int VIDEO_interval = atoi(argv[3]);
    if(VIDEO_interval <= 0) {
        perror("VIDEO Interval must be > 0");
        exit(EXIT_FAILURE);
    }
    #ifdef DEBUG
        printf("VIDEO Interval = %d\n", VIDEO_interval);
    #endif

    int MUSIC_interval = atoi(argv[4]);
    if(MUSIC_interval <= 0) {
        perror("MUSIC Interval must be > 0");
        exit(EXIT_FAILURE);
    }
    #ifdef DEBUG
        printf("MUSIC Interval = %d\n", MUSIC_interval);
    #endif

    int SOCIAL_interval = atoi(argv[5]);
    if(SOCIAL_interval <= 0) {
        perror("SOCIAL Interval must be > 0");
        exit(EXIT_FAILURE);
    }
    #ifdef DEBUG
        printf("SOCIAL Interval = %d\n", SOCIAL_interval);
    #endif

    int data_to_reserve = atoi(argv[6]);
    if(data_to_reserve <= 0) {
        perror("Data_to_Reserve must be > 0");
        exit(EXIT_FAILURE);
    }
    #ifdef DEBUG
        printf("Data to Reserve = %d\n", data_to_reserve);
    #endif

    /* Completed relative path to named semaphore */
    if(sprintf(mq_named_sem_path, "%s%d", MQ_NAMED_SEMAPHORE_DIR_PATH, MOBILE_USER_ID) < 0) error("Creating mq_named_sem_path string");

    /* Initializing user pipe named semaphore (mutex) */
    user_pipe_mutex = sem_open(USER_PIPE_MUTEX_PATH, 0);
    if (user_pipe_mutex == SEM_FAILED) error("Opening user pipe named semaphore (mutex)");
    userPipeMutexOpened = true;

    /* Initializing message queue named semaphore */
    mq_named_sem = sem_open(mq_named_sem_path, O_CREAT, 0666, 0);
    if (mq_named_sem == SEM_FAILED) error("Opening message queue named semaphore");
    mqNamedSemCreated = true;

    /* Opening the pipe for writing */
    user_pipe_fd = open(USER_PIPE_PATH, O_WRONLY);
    if (user_pipe_fd == -1) error("Opening user pipe for writing");
    userPipeFdOpened = true;

    /* Creating the message queue key */
    int mq_key = ftok(MQ_KEY_PATH, MQ_KEY_ID);
    if(mq_key == -1) error("Creating message queue key");

    /* Opening the message queue for reading */
    mq_id = msgget(mq_key, 0400);  // 0400 --> read-only permissions
    if (mq_id == -1) error("Getting message queue id");

    /* Sending register message */
    send_reg_message(initial_plafond);

    int t;
    int i = 0;
    long long time_S = 0, time_M = 0, time_V = 0;
    while(i < max_autorizations_requests) {
        /* Tries wait to see if there is a message in message queue */
        if(sem_trywait(mq_named_sem) == 0) {
            if(receive_message() == -1) break;
        }

        t = get_millis();
        if((t - time_S) >= SOCIAL_interval) {
            send_social_req(data_to_reserve);
            time_S = get_millis();
            i++;
        }
        else if((t - time_M) >= MUSIC_interval) {
            send_music_req(data_to_reserve);
            time_M = get_millis();
            i++;
        }
        else if((t - time_V) >= VIDEO_interval) {
            send_video_req(data_to_reserve);
            time_V = get_millis();
            i++;
        }
        else usleep(500); // sleep for 500 microseconds = 0.5 milliseconds
    }

    /* Frees all resorces */
    free_resorces();

    return 0;
}

/**
 * Receives a message from message queue and prints it. Return -1 if 100% plafond reached and 0 otherwise.
 */
int receive_message() {
    if(msgrcv(mq_id, &message, sizeof(message), MOBILE_USER_ID, NULL) == -1) error("Receiving message from message queue");
    puts(message.msg_text);
    if(strcmp(message.msg_text, "ALERT: 100%% Plafond reached!") == 0) return -1;
    else return 0;
}

/**
 * Sends the registation message with the mobile user ID and initial plafond.
 */
void send_reg_message(int initial_plafond) {
    sem_wait(user_pipe_mutex);
    if(sprintf(command, "%d#%d", MOBILE_USER_ID, initial_plafond) < 0) error("creating register message");
    if(write(user_pipe_fd, command, strlen(command) + 1) == -1) error("sending register message to user pipe");
    sem_post(user_pipe_mutex);
}

/**
 * Sends a social request to user pipe.
 */
void send_social_req(int data_to_reserve) {
    sem_wait(user_pipe_mutex);
    if(sprintf(command, "%d#SOCIAL#%d", MOBILE_USER_ID, data_to_reserve) < 0) error("creating social request message");
    if(write(user_pipe_fd, command, strlen(command) + 1) == -1) error("sending social request message to user pipe");
    sem_post(user_pipe_mutex);
}

/**
 * Sends a music request to user pipe.
 */
void send_music_req(int data_to_reserve) {
    sem_wait(user_pipe_mutex);
    if(sprintf(command, "%d#MUSIC#%d", MOBILE_USER_ID, data_to_reserve) < 0) error("creating music request message");
    if(write(user_pipe_fd, command, strlen(command) + 1) == -1) error("sending music request message to user pipe");
    sem_post(user_pipe_mutex);
}

/**
 * Sends a video request to user pipe.
 */
void send_video_req(int data_to_reserve) {
    sem_wait(user_pipe_mutex);
    if(sprintf(command, "%d#VIDEO#%d", MOBILE_USER_ID, data_to_reserve) < 0) error("creating video request message");
    if(write(user_pipe_fd, command, strlen(command) + 1) == -1) error("sending video request message to user pipe");
    sem_post(user_pipe_mutex);
}

/**
 * Gets the milliseconds.
 */
long long get_millis() {
    struct timespec ts;

    /* Getting current time */
    clock_gettime(CLOCK_MONOTONIC, &ts);

    /* Calculating milliseconds */
    long long milliseconds = ts.tv_sec * 1000LL + ts.tv_nsec / 1000000;

    return milliseconds;
}

/**
 * Closes all the semaphores already created.
 */
void close_sems() {
    if(userPipeMutexOpened) sem_close(user_pipe_mutex);
    if(mqNamedSemCreated) {
        sem_close(mq_named_sem);
        sem_unlink(mq_named_sem_path);
    }
}

/**
 * Frees resorces (mutex and pipe file descriptor)
 */
void free_resorces() {
    /* Closes the created semaphores */
    close_sems();

    /* Closes the user pipe file descriptor */
    if(userPipeFdOpened) close(user_pipe_fd);
}

/**
 * Handles the SIGINT signal.
 */
void handle_sigint(int sig) {
    free_resorces();

    printf("SIGINT (%d) received. Closing Mobile User...\n", sig);
    exit(EXIT_SUCCESS);
}

/**
 * Liberates all the resorces and prints error message.
 */
void error(char* str_to_print) {
    free_resorces();

    fprintf(stderr, "Error: %s\n", str_to_print);
    exit(EXIT_FAILURE);
}


