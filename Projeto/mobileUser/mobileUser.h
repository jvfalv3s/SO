/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#ifndef MOBILEUSER_H
#define MOBILEUSER_H

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
#include <signal.h>
#include <ctype.h>
#include <time.h>
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
#define MQ_KEY_PATH "/tmp/message_queue"
#define MQ_KEY_ID 'a'
/* Path of the USER PIPE */
#define USER_PIPE_PATH "/tmp/user_pipe"
/* Path to USER PIPE NAMED MUTEX SEMAPHORE */
#define USER_PIPE_MUTEX_PATH "user_pipe_mutex_sem"
/* Path to MESSAGE QUEUE NAMED SEMAPHORE directory */
#define MQ_NAMED_SEMAPHORE_DIR_PATH "/tmp/mq_sem_"

/* Message from message queue struct */
typedef struct mq_message {
    long mgg_type;
    char msg_text [MAX_CHAR_MESSAGE_AMMOUNT];
}mq_message;

int mq_id;  // Message queue id

/**
 * Receives a message from message queue and prints it. Return -1 if 100% plafond reached and 0 otherwise.
 */
int receive_message();

/**
 * Sends the registation message with the mobile user ID and initial plafond.
 */
void send_reg_message(int initial_plafond);

/**
 * Sends a social request to user pipe.
 */
void send_social_req(int data_to_reserve);

/**
 * Sends a music request to user pipe.
 */
void send_music_req(int data_to_reserve);

/**
 * Sends a video request to user pipe.
 */
void send_video_req(int data_to_reserve);

/**
 * Gets the milliseconds.
 */
long long get_millis();

/**
 * Closes all the semaphores already created.
 */
void close_sems();

/**
 * Frees resorces (mutex and pipe file descriptor)
 */
void free_resorces();

/**
 * Handles the SIGINT signal.
 */
void handle_sigint(int sig);

/**
 * Frees all the resorces and prints error message.
 */
void error(char* str_to_print);

#endif
