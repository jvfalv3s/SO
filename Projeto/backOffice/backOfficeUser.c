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

/* Path of the BACK PIPE */
#define BACK_PIPE_PATH "../tmp/FIFO/back_pipe"
/* Path of the MESSAGE QUEUE */
#define MESSAGE_QUEUE_PATH "../tmp/FIFO/message_queue"

/* Initializing some usefull variables */
char* BACKOFFICE_USER_ID = "1";
char command [MAX_CHAR_COMMAND_AMMOUNT];
int back_pipe_fd;
mqd_t mq;

/**
 * Main function.
 */
int main() {
    /* SIGINT handling */
    signal(SIGINT, handle_sigint);

    /* Opening the pipe for writing */
    back_pipe_fd = open(BACK_PIPE_PATH, O_WRONLY);
    if (back_pipe_fd == -1) {
        perror("Opening user pipe for writing");
        exit(EXIT_FAILURE);
    }

    /* Opening the message queue for reading */
    mq = mq_open(MESSAGE_QUEUE_PATH, O_RDONLY);
    if(mq == (mqd_t)-1) {
        close(back_pipe_fd);
        perror("Opening message queue for reading");
        exit(EXIT_FAILURE);
    }

    /* Loop to read and verify commands of the BackOffice User */
    char* token;
    char commandAux[MAX_CHAR_COMMAND_AMMOUNT + 20];
    while(1) {
        /*
         * TODO:
         * --> trywait for possible message in message mq and displays it in the screen
         */

        if(sprintf(command, "Waitting new command...\n") < 0) error("creating waitting new command message");
        puts(command);

        /* Waits a new command to be written */
        fgets(command, sizeof(command), stdin);

        /* Verification of the backoffice user ID */
        token = strtok(command, "#");
        if(token == NULL) {  // Verifyes if the command is valid
            if(sprintf(command, "invalid command\n") < 0) error("creating invalid command message");
            puts(command);
            continue;
        }
        else if(strcmp(token, BACKOFFICE_USER_ID) != 0) {  // Verifyes if the backoffice user id is compatible
            if(sprintf(command, "invalid backoffice user id\n") < 0) error("creating invalid backoffice user id message");
            puts(command);
            continue;
        }

        /* Verification of the other argumments */
        token = strtok(NULL, "#");
        if(token == NULL) {  // Verifyes if the command is valid
            if(sprintf(command, "invalid command\n") < 0) error("creating invalid command message");
            puts(command);
            continue;
        }
        else if(!((strcmp(token, "data_stats") == 0) || (strcmp(token, "reset") == 0))) {  // Verifyes if the second argumment is valid
            if(sprintf(command, "invalid command\n") < 0) error("creating invalid command message");
            puts(command);
            continue;
        }
            
        /* Sending command to BACK PIPE */
        if(write(back_pipe_fd, command, strlen(command) + 1) == -1) error("sending command to back pipe");
        puts("Command sent!\n");

        /* If the command was to reset just wait for a new command because there will be nothing to display */
        if(strcmp(token, "reset") == 0) continue;
        else {
            /*
             * TODO: 
             * --> wait message from the mq for command "data_stats"
             */
        }
    }

    /* Closes the back pipe file descriptor */
    close(back_pipe_fd);

    return 0;
}

/**
 * Free all the resorces.
 */
void free_resorces() {
    /* Closes the user pipe file descriptor */
    close(back_pipe_fd);

    /* Closes the message queue */
    mq_close(mq);
}

/**
 * Handles the SIGINT signal.
 */
void handle_sigint(int sig) {
    /* Closes the back pipe file descriptor */
    close(back_pipe_fd);

    printf("SIGINT (%d) received. Closing backoffice User...\n", sig);
    exit(EXIT_SUCCESS);
}

/**
 * Frees all the resorces and prints error message.
 */
void error(char* str_to_print) {
    /* Closes the back pipe file descriptor */
    close(back_pipe_fd);

    fprintf(stderr, "Error: %s\n", str_to_print);
    exit(EXIT_FAILURE);
}
