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

/*
#define BACK_PIPE "back_pipe"
#define MESSAGE_QUEUE_KEY 1234
*/

/* Max characters a command can have */
#define MAX_CHAR_COMMAND_AMMOUNT 100

/* Path of the BACK PIPE */
#define BACK_PIPE_PATH "../tmp/FIFO/back_pipe"

/* Initializing some usefull variables */
char BACKOFFICE_USER_ID[6];
char command [MAX_CHAR_COMMAND_AMMOUNT];
int back_pipe_fd;

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

    /* Loop to read and verify commands of the BackOffice User */
    char* token;
    char commandAux[MAX_CHAR_COMMAND_AMMOUNT + 20];
    while(1) {
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
        else if((BACKOFFICE_USER_ID != NULL) && (strcmp(token, BACKOFFICE_USER_ID) != 0)) {  // Verifyes if the backoffice user id exists and/or is compatible
            if(sprintf(command, "invalid backoffice user id\n") < 0) error("creating invalid backoffice user id message");
            puts(command);
           
            continue;
        }
        strcpy(BACKOFFICE_USER_ID, token);

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
    }

    /* Closes the back pipe file descriptor */
    close(back_pipe_fd);

    return 0;
}

/*
void startBackOfficeUser() {

    // Abre o named pipe BACK_PIPE para escrita
    int back_pipe_fd = open(BACK_PIPE, O_WRONLY);
    if (back_pipe_fd == -1) {
        perror("Erro ao abrir o named pipe BACK_PIPE");
        exit(EXIT_FAILURE);
    }

    // Cria ou conecta à Message Queue
    int msgqid;
    if ((msgqid = msgget(MESSAGE_QUEUE_KEY, IPC_CREAT | 0666)) == -1) {
        perror("Erro ao criar/conectar a Message Queue");
        exit(EXIT_FAILURE);
    }

    // Loop para receber estatísticas periódicas e solicitar estatísticas assíncronas
    while (!should_exit) { // Saia do loop se should_exit for verdadeiro
        // Implemente aqui a lógica para receber estatísticas periódicas e solicitar estatísticas assíncronas

        sleep(10); // Aguarda 10 segundos antes de continuar para simular o recebimento periódico de estatísticas
    }

    // Fecha o pipe BACK_PIPE
    close(back_pipe_fd);
}
*/

/*
// Função para receber estatísticas agregadas periodicamente
void receive_aggregated_statistics() {
    // Implemente a lógica para receber estatísticas agregadas da fila de mensagens
}
*/

/*
// Função para solicitar estatísticas de forma assíncrona ao Authorization Requests Manager
void request_statistics(int back_pipe_fd) {
    char request[] = "STATS"; // Exemplo de solicitação de estatísticas
    write(back_pipe_fd, request, strlen(request) + 1);
}
*/

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
 * LIBERATE all the resorces and prints error message.
 */
void error(char* str_to_print) {
    /* Closes the back pipe file descriptor */
    close(back_pipe_fd);

    if(fprintf(stderr, "Error: %s\n", str_to_print) < 0) exit(EXIT_FAILURE);
    exit(EXIT_FAILURE);
}
