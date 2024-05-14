/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#ifndef BACKOFFICEUSER_H
#define BACKOFFICEUSER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>

/* Comment this line to don't show debug messages */
#define DEBUG

/* Max characters a command can have */
#define MAX_CHAR_COMMAND_AMMOUNT 100
/* Max characters a message can have */
#define MAX_CHAR_MESSAGE_AMMOUNT 200
/* ftok arguments to create the message queue key */
#define MQ_KEY_PATH "/tmp/message_queue"
#define MQ_KEY_ID 'a'
/* Path of the BACK PIPE */
#define BACK_PIPE_PATH "back_pipe"
/* Path to MESSAGE QUEUE NAMED SEMAPHORE that indicates about periodic stats */
#define MQ_NAMED_SEMAPHORE_P_PATH "/tmp/back_sem_P"
/* Path to MESSAGE QUEUE NAMED SEMAPHORE that indicates about stats response */
#define MQ_NAMED_SEMAPHORE_R_PATH "/tmp/back_sem_R"

/* Message from message queue struct */
typedef struct mq_message {
    long mgg_type;
    char msg_text [MAX_CHAR_MESSAGE_AMMOUNT];
}mq_message;

int mq_id;

/**
 * Receives a message from message queue and prints it.
 */
void receive_message();

/**
 * Free all the resorces.
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