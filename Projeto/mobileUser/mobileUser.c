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
#include <pthread.h>
#include <mqueue.h>

/* Comment this line to don't show debug messages */
#define DEBUG

/* Max characters a command can have */
#define MAX_CHAR_COMMAND_AMMOUNT 20
/* Path of the USER PIPE */
#define USER_PIPE_PATH "../tmp/FIFO/user_pipe"
/* Path of the MESSAGE QUEUE */
#define MESSAGE_QUEUE_PATH "../tmp/FIFO/message_queue"

/* Initializing some usefull variables */
int MOBILE_USER_ID;
char command[MAX_CHAR_COMMAND_AMMOUNT];
int user_pipe_fd;
/* ---------------------------------------------------------------------------------------------------------------------------
 * MUTEX AQUI NAO FAZ SENTIDO, TROCAR POR NAMED SEMAPHORE? assim todos os mobile users poderiam aceder e n se atropelar ao
 * enviar para o pipe (podia controlar tmb a quantidade de pedidos e no receiver e mandar 'mandar parar' quando tivesse cheio
 * em outras palavras n deixar avançar com o semafero)
 */
pthread_mutex_t mutex;  
mqd_t mq;

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

    /* Opening the pipe for writing */
    user_pipe_fd = open(USER_PIPE_PATH, O_WRONLY);
    if (user_pipe_fd == -1) {
        perror("Opening user pipe for writing");
        exit(EXIT_FAILURE);
    }

    /* Opening the message queue for reading */
    mq = mq_open(MESSAGE_QUEUE_PATH, O_RDONLY);
    if(mq == (mqd_t)-1) {
        close(user_pipe_fd);
        perror("Opening message queue for reading");
        exit(EXIT_FAILURE);
    }

    /* Initializing mutex */
    if(pthread_mutex_init(&mutex, NULL) != 0) {
        close(user_pipe_fd);
        perror("initializing mutex");
        exit(EXIT_FAILURE);
    }

    /* Sending register message */
    send_reg_message(initial_plafond);

    int t;
    int i = 0;
    long long time_S = 0, time_M = 0, time_V = 0;
    while(i < max_autorizations_requests) {
        
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
 * Sends the registation message with the mobile user ID and initial plafond.
 */
void send_reg_message(int initial_plafond) {
    pthread_mutex_lock(&mutex);
    if(sprintf(command, "%d#%d", MOBILE_USER_ID, initial_plafond) < 0) error("creating register message");
    if(write(user_pipe_fd, command, strlen(command) + 1) == -1) error("sending register message to user pipe");
    pthread_mutex_unlock(&mutex);
}

/**
 * Sends a social request to user pipe.
 */
void send_social_req(int data_to_reserve) {
    pthread_mutex_lock(&mutex);
    if(sprintf(command, "%d#SOCIAL#%d", MOBILE_USER_ID, data_to_reserve) < 0) error("creating social request message");
    if(write(user_pipe_fd, command, strlen(command) + 1) == -1) error("sending social request message to user pipe");
    pthread_mutex_unlock(&mutex); 
}

/**
 * Sends a music request to user pipe.
 */
void send_music_req(int data_to_reserve) {
    pthread_mutex_lock(&mutex);
    if(sprintf(command, "%d#MUSIC#%d", MOBILE_USER_ID, data_to_reserve) < 0) error("creating music request message");
    if(write(user_pipe_fd, command, strlen(command) + 1) == -1) error("sending music request message to user pipe");
    pthread_mutex_unlock(&mutex); 
}

/**
 * Sends a video request to user pipe.
 */
void send_video_req(int data_to_reserve) {
    pthread_mutex_lock(&mutex);
    if(sprintf(command, "%d#VIDEO#%d", MOBILE_USER_ID, data_to_reserve) < 0) error("creating video request message");
    if(write(user_pipe_fd, command, strlen(command) + 1) == -1) error("sending video request message to user pipe");
    pthread_mutex_unlock(&mutex); 
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
 * Frees resorces (mutex and pipe file descriptor)
 */
void free_resorces() {
    /* Closes the user pipe file descriptor */
    close(user_pipe_fd);

    /* Closes the message queue */
    mq_close(mq);

    /* Destroing mutex */
    pthread_mutex_destroy(&mutex);
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



void enganometa1() {
    // if(sprintf(command, "waitting new command...\n") < 0) error("creating waitting new command message");
        // puts(command);

        // /* waits a new command to be written */
        // fgets(command, sizeof(command), stdin);

        // /* verification of the mobile user id */
        // token = strtok(strcpy(commandaux, command), "#");
        // if(token == null || isdigit(token) == 0) {  // verifyes if the command is valid
        //     if(sprintf(command, "invalid command\n") < 0) error("creating invalid command message");
        //     puts(command);
        //     i--;
        //     continue;
        // }
        // else if((logged_in != 0) && (strcmp(token, mobile_user_id) != 0)) {  // verifies if the mobile user id exists and/or is compatible
        //     if(sprintf(command, "invalid mobile user id\n") < 0) error("creating invalid mobile user id message");
        //     puts(command);
        //     i--;
        //     continue;
        // }
        // else if(logged_in == 0) {  // if not logged in, login and sets mobile user id
        //     strcpy(mobile_user_id, token);
        //     logged_in = 1;
        // }

        // /* verification of the other argumments */
        // token = strtok(null, "#");
        // if(token == null) {  // verifyes if the command is valid
        //     if(sprintf(command, "invalid command\n") < 0) error("creating invalid command message");
        //     puts(command);
        //     i--;
        //     continue;
        // }
        // else if(strtok(null, "#") != null) {  // verifyes if there exists a third argument
        //     if(!((strcmp(token, "video") == 0) || (strcmp(token, "music") == 0) || (strcmp(token, "social") == 0))) {  // verifyes if the second argumment is valid
        //         if(sprintf(command, "invalid service id\n") < 0) error("creating service id message");
        //         puts(command);
        //         i--;
        //         continue;
        //     }
        // }
        // else if(isdigit(token) == 0) {  // verifies, in case of just 2 arguments, if the second is a number
        //     if(sprintf(command, "invalid second argument\n") < 0) error("creating invalid command message");
        //     puts(command);
        //     i--;
        //     continue;
        // }

        // /* sending command to user pipe */
        // if(write(user_pipe_fd, command, strlen(command) + 1) == -1) error("sending command to user pipe");
        // puts("command sent!\n");
}

