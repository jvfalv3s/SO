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
#include <stdio.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <stdbool.h>
#include "backOfficeUser.h"

/* Max characters a command can have */
#define MAX_CHAR_COMMAND_AMMOUNT 100
/* Max characters a message can have */
#define MAX_CHAR_MESSAGE_AMMOUNT 200
/* ftok arguments to create the message queue key */
#define MQ_KEY_PATH "/message_queue"
#define MQ_KEY_ID 'a'
/* Path of the BACK PIPE */
#define BACK_PIPE_PATH "back_pipe"
/* Path to MESSAGE QUEUE NAMED SEMAPHORE that indicates about periodic stats */
#define MQ_NAMED_SEMAPHORE_P_PATH "back_sem_P"
/* Path to MESSAGE QUEUE NAMED SEMAPHORE that indicates about stats response */
#define MQ_NAMED_SEMAPHORE_R_PATH "back_sem_R"

/* Message from message queue struct */
typedef struct mq_message {
    long mgg_type;
    char msg_text [MAX_CHAR_MESSAGE_AMMOUNT];
};

/* Initializing some usefull variables */
char* BACKOFFICE_USER_ID = "1";
char command [MAX_CHAR_COMMAND_AMMOUNT];
struct mq_message message;                 // Message from message queue
int back_pipe_fd;
sem_t *mq_named_sem_p, *mq_named_sem_r;
int mq_id;
bool backPipeFdOpened = false;
bool mqNamedSemPCreated = false, mqNamedSemRCreated = false;

/**
 * Main function.
 */
int main() {
    /* SIGINT handling */
    signal(SIGINT, handle_sigint);

    /* Initializing message queue named semaphore that indicates about periodic stats */
    mq_named_sem_p = sem_open(MQ_NAMED_SEMAPHORE_P_PATH, O_CREAT, 0666, 0);
    if (mq_named_sem_p == SEM_FAILED) error("Opening message queue named semaphore P");
    mqNamedSemPCreated = true;

    /* Initializing message queue named semaphore that indicates about stats response */
    mq_named_sem_r = sem_open(MQ_NAMED_SEMAPHORE_R_PATH, O_CREAT, 0666, 0);
    if (mq_named_sem_r == SEM_FAILED) error("Opening message queue named semaphore R");
    mqNamedSemRCreated = true;

    /* Opening the pipe for writing */
    back_pipe_fd = open(BACK_PIPE_PATH, O_WRONLY);
    if (back_pipe_fd == -1) error("Opening back office user pipe for writing");
    backPipeFdOpened = true;

    /* Creating the message queue key */
    int mq_key = ftok(MQ_KEY_PATH, MQ_KEY_ID);
    if(mq_key == -1) error("Creating message queue key");

    /* Opening the message queue for reading */
    mq_id = msgget(mq_key, 0400);  // 0400 --> read-only permissions
    if (mq_id == -1) error("Getting message queue id");

    /* Loop to read and verify commands of the BackOffice User */
    char* token;
    char commandAux[MAX_CHAR_COMMAND_AMMOUNT + 20];
    while(true) {
        while(sem_trywait(mq_named_sem_p) == 0) receive_message();

        if(sprintf(command, "Waitting new command...\n") < 0) error("Creating waitting new command message");
        puts(command);

        /* Waits a new command to be written */
        fgets(command, sizeof(command), stdin);

        /* Verification of the backoffice user ID */
        token = strtok(command, "#");
        if(token == NULL) {  // Verifyes if the command is valid
            if(sprintf(command, "invalid command\n") < 0) error("Creating invalid command message");
            puts(command);
            continue;
        }
        else if(strcmp(token, BACKOFFICE_USER_ID) != 0) {  // Verifyes if the backoffice user id is compatible
            if(sprintf(command, "invalid backoffice user id\n") < 0) error("Creating invalid backoffice user id message");
            puts(command);
            continue;
        }

        /* Verification of the second argumment */
        token = strtok(NULL, "");
        if(token == NULL) {  // Verifyes if the command is valid
            if(sprintf(command, "invalid command\n") < 0) error("Creating invalid command message");
            puts(command);
            continue;
        }
        else if(!((strcmp(token, "data_stats") == 0) || (strcmp(token, "reset") == 0))) {  // Verifyes if the second argumment is valid
            if(sprintf(command, "invalid command\n") < 0) error("Creating invalid command message");
            puts(command);
            continue;
        }
            
        /* Sending command to BACK PIPE */
        if(write(back_pipe_fd, command, strlen(command) + 1) == -1) error("Sending command to back pipe");
        puts("Command sent!\n");

        /* If the command was to reset just wait for a new command because there will be nothing to display */
        if(strcmp(token, "reset") == 0) continue;
        /* Otherwises waits for a response */
        else {
            sem_wait(mq_named_sem_r);
            receive_message();
        }
    }

    free_resorces();

    return 0;
}

/**
 * Receives a message from message queue and prints it.
 */
void receive_message() {
    if(msgrcv(mq_id, &message, sizeof(message), 1, NULL) == -1) error("Receiving message from message queue");
    puts(message.msg_text);
}

/**
 * Free all the resorces.
 */
void free_resorces() {
    /* Closes the back office user pipe file descriptor */
    if(backPipeFdOpened) close(back_pipe_fd);

    /* Closes and unlink the semaphores */
    if(mqNamedSemPCreated) {
        sem_close(mq_named_sem_p);
        sem_unlink(MQ_NAMED_SEMAPHORE_P_PATH);
    }
    if(mqNamedSemRCreated) {
        sem_close(mq_named_sem_r);
        sem_unlink(MQ_NAMED_SEMAPHORE_R_PATH);
    }
}

/**
 * Handles the SIGINT signal.
 */
void handle_sigint(int sig) {
    /* Closes the back pipe file descriptor */
    free_resorces();

    printf("SIGINT (%d) received. Closing backoffice User...\n", sig);
    exit(EXIT_SUCCESS);
}

/**
 * Frees all the resorces and prints error message.
 */
void error(char* str_to_print) {
    free_resorces();

    fprintf(stderr, "Error: %s\n", str_to_print);
    exit(EXIT_FAILURE);
}
