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

/* Comment this line to don't show debug messages */
#define DEBUG

/* Max characters a command can have */
#define MAX_CHAR_COMMAND_AMMOUNT 1000
/* Path of the USER PIPE */
#define USER_PIPE_PATH "../tmp/FIFO/user_pipe"

/* Initializing some usefull variables */
char MOBILE_USER_ID[6];
char command[MAX_CHAR_COMMAND_AMMOUNT];
int user_pipe_fd;

/*
#define VIDEO_STREAMING_QUEUE_KEY 5678
#define MESSAGE_QUEUE_KEY 1234
*/

/* Possivel solução para pipes
// Estrutura para as mensagens na fila
struct message_buffer {
    long message_type;
    char message_text[100];
};
*/

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

    /* Opening the pipe for writing */
    user_pipe_fd = open(USER_PIPE_PATH, O_WRONLY);
    if (user_pipe_fd == -1) {
        perror("Opening user pipe for writing");
        exit(EXIT_FAILURE);
    }

    /*
    // Criar ou obter a fila de mensagens de vídeo
    int video_queue_id = msgget(VIDEO_STREAMING_QUEUE_KEY, IPC_CREAT | 0666);
    if (video_queue_id == -1) {
        perror("Erro ao criar ou obter a fila de mensagens de vídeo");
        exit(EXIT_FAILURE);
    }

    // Criar ou obter a fila de mensagens
    int message_queue_id = msgget(MESSAGE_QUEUE_KEY, IPC_CREAT | 0666);
    if (message_queue_id == -1) {
        perror("Erro ao criar ou obter a fila de mensagens");
        exit(EXIT_FAILURE);
    }
    */

    /* Loop to read and verify commands of the Mobile User */
    char* token;
    char commandAux[MAX_CHAR_COMMAND_AMMOUNT + 20];
    short logged_in = 0;
    for (int i = 0; i < max_autorizations_requests; ++i) {
        if(sprintf(command, "Waitting new command...\n") < 0) error("creating waitting new command message");
        puts(command);

        /* Waits a new command to be written */
        fgets(command, sizeof(command), stdin);

        /* Verification of the mobile user ID */
        token = strtok(strcpy(commandAux, command), "#");
        if(token == NULL || isdigit(token) == 0) {  // Verifyes if the command is valid
            if(sprintf(command, "Invalid command\n") < 0) error("creating invalid command message");
            puts(command);
            i--;
            continue;
        }
        else if((logged_in != 0) && (strcmp(token, MOBILE_USER_ID) != 0)) {  // Verifies if the mobile user id exists and/or is compatible
            if(sprintf(command, "Invalid mobile user ID\n") < 0) error("creating invalid mobile user id message");
            puts(command);
            i--;
            continue;
        }
        else if(logged_in == 0) {  // If not logged in, login and sets mobile user id
            strcpy(MOBILE_USER_ID, token);
            logged_in = 1;
        }

        /* Verification of the other argumments */
        token = strtok(NULL, "#");
        if(token == NULL) {  // Verifyes if the command is valid
            if(sprintf(command, "Invalid command\n") < 0) error("creating invalid command message");
            puts(command);
            i--;
            continue;
        }
        else if(strtok(NULL, "#") != NULL) {  // Verifyes if there exists a third argument
            if(!((strcmp(token, "VIDEO") == 0) || (strcmp(token, "MUSIC") == 0) || (strcmp(token, "SOCIAL") == 0))) {  // Verifyes if the second argumment is valid
                if(sprintf(command, "Invalid service ID\n") < 0) error("creating service ID message");
                puts(command);
                i--;
                continue;
            }
        }
        else if(isdigit(token) == 0) {  // Verifies, in case of just 2 arguments, if the second is a number
            if(sprintf(command, "Invalid second argument\n") < 0) error("creating invalid command message");
            puts(command);
            i--;
            continue;
        }

        /* Sending command to USER PIPE */
        if(write(user_pipe_fd, command, strlen(command) + 1) == -1) error("sending command to user pipe");
        puts("Command sent!\n");
        
        /*
        // Gerar e enviar pedidos de serviço
        send_data_request(user_pipe_fd, video_queue_id, initial_balance);

        // Receber alertas relacionados com os consumos dos vários serviços
        receive_alerts(message_queue_id);

        // Esperar um intervalo de tempo antes de enviar outro pedido (por exemplo, 1 segundo)
        sleep(1);
        */
    }

    /* Closes the user pipe file descriptor */
    close(user_pipe_fd);

    /*
    // Remover a fila de mensagens de vídeo
    if (msgctl(video_queue_id, IPC_RMID, NULL) == -1) {
        perror("Erro ao encerrar a fila de mensagens de vídeo");
        exit(EXIT_FAILURE);
    }

    // Remover a fila de mensagens
    if (msgctl(message_queue_id, IPC_RMID, NULL) == -1) {
        perror("Erro ao encerrar a fila de mensagens");
        exit(EXIT_FAILURE);
    }
    */

    return 0;
}

/*
void startMobileUser(char* command) {
    // Cria ou conecta à Message Queue
    int msgqid;
    if ((msgqid = msgget(MESSAGE_QUEUE_KEY, IPC_CREAT | 0666)) == -1) {
        perror("Erro ao criar/conectar a Message Queue");
        exit(EXIT_FAILURE);
    }

    // Envia mensagem de registo inicial
    dprintf(user_pipe_fd, "%s#%s\n", userID, receiveID);
}
*/

/**
 * Handles the SIGINT signal.
 */
void handle_sigint(int sig) {
    /* Closes the user pipe file descriptor */
    close(user_pipe_fd);

    printf("SIGINT (%d) received. Closing Mobile User...\n", sig);
    exit(EXIT_SUCCESS);
}

/**
 * Liberates all the resorces and prints error message.
 */
void error(char* str_to_print) {
    /* Closes the user pipe file descriptor */
    close(user_pipe_fd);

    if(fprintf(stderr, "Error: %s\n", str_to_print) < 0) exit(EXIT_FAILURE);
    exit(EXIT_FAILURE);
}