/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdbool.h>
#include "./LogFileManager/LogFileManager.h"
#include "./AutorizationReqManager/AutorizationReqManager.h"
#include "./MonitorEngine/MonitorEngine.h"

/* Comment this line to don't show debug messages */
#define DEBUG

/* Shared memory important definitions */
#define MAX_USERS_SHM 20      // Max number users (defines shm size)
#define SHM_PATH "./tmp/shm"  // Path to shm file

void shmClose();
void killProcess();
void handle_sigint();
void handle_sigquit();
void endSys();

/* Sharerd memory structur */
typedef struct user {
    int id;
    int current_plafond;
    int max_plafond;
} user;

typedef struct auth_eng{
    pid_t pid;
    int pipe_read_fd;
    int pipe_write_fd;
    bool busy;
    time_t l_request_time;
}auth_eng;

typedef struct shm_struct {
    struct user[MAX_USERS_SHM];
    struct auth_eng* auth_engs;
    int n_users;
    int n_auth_engs;
}shm_struct;

/* Initializations */
pid_t ARM_PID, ME_PID;  // System processes PIDs
bool AutReqManCreated = false, MonEngCreated = false;  // System processes creation status
int shm_fd;                  // Shared memory file descriptor
struct shm_struct* shm_ptr;  // Shared memory pointer
int shm_size = sizeof(struct shm_struct);  // Shared memory size

/**
 * Main Function.
 */
int main(int argc, char* argv[]) {
    /* Check if the arguments of execution are correct */
    if(argc != 2) {
        fprintf(stderr, "Not enought arguments: Use: %s <config-file-name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Creates a new log file and log the creation of the system manager process */
    creatLogFile();
    writeLog("PROCESS SYSTEM_MANAGER CREATED");

    /* Create a new shared memory object and maping it into address space of the process */
    if((shm_fd = shm_open(SHM_PATH, O_CREAT | O_RDWR, 0666)) == -1) {
        error("SHARED MEMORY OPEN FAILED");
        exit(EXIT_FAILURE);
    }
    if(ftruncate(shm_fd, shm_size) == -1) {
        error("FTRUNCATE FAILED");
        exit(EXIT_FAILURE);
    }
    shm_ptr = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(shm_ptr == MAP_FAILED) {
        error("MEMORY MAP FAILED");
        exit(EXIT_FAILURE);
    }

    /* Initialization of the variables obtained from the config file */
    int MOBILE_USERS;
    int QUEUE_POS;
    int AUTH_SERVERS_MAX;
    int AUTH_PROC_TIME;
    int MAX_VIDEO_WAIT;
    int MAX_OTHERS_WAIT;

    FILE* f;
    char* buf = (char*) malloc(sizeof(char)*100);
    int i = 0;

    /* Reads the config file and obtain all the configurations from it and also verifies if everythin is valid */
    #ifdef DEBUG
        printf("Start reading file...\n");
    #endif
    if((f = fopen(argv[1], "r")) == NULL) {
        error("Opening the config file");
        shmClose();
        exit(EXIT_FAILURE);
    }
    while(fgets(buf, sizeof(buf), f)) {
        int atribute = atoi(buf);  // Transforming the strings in the file into integers
        printf("%d",atribute);
        /* The config file must follow an order so we use a counter to know which line we are */
        switch (i)
        {
        case 0:
            if(atribute <= 0) {
                error("MOBILE_USERS <= 0");
                shmClose();
                exit(EXIT_FAILURE);
            }
            MOBILE_USERS = atribute;
            #ifdef DEBUG
                printf("MOBILE_USERS = %d\n", MOBILE_USERS);
            #endif
            break;
        case 1: // QUEUE_POS atribute
            if(atribute < 0) {
                error("QUEUE_POS < 0");
                shmClose();
                exit(EXIT_FAILURE);
            }
            QUEUE_POS = atribute;
            #ifdef DEBUG
                printf("QUEUE_POS = %d\n", QUEUE_POS);
            #endif
            break;
        case 2: // AUTH_SERVERS_MAX atribute
            if(atribute < 1) {
                error("AUTH_SERVERS_MAX < 1");
                shmClose();
                exit(EXIT_FAILURE);
            }
            AUTH_SERVERS_MAX = atribute;
            #ifdef DEBUG
                printf("AUTH_SERVERS_MAX = %d\n", AUTH_SERVERS_MAX);
            #endif
            break;
        case 3: // AUTH_PROC_TIME atribute
            if(atribute < 0) {
                error("AUTH_PROC_TIME < 0");
                shmClose();
                exit(EXIT_FAILURE);
            }
            AUTH_PROC_TIME = atribute;
            #ifdef DEBUG
                printf("AUTH_PROC_TIME = %d\n", AUTH_PROC_TIME);
            #endif
            break;
        case 4: // MAX_VIDEO_WAIT atribute
            if(atribute < 1) {
                error("MAX_VIDEO_WAIT < 1");
                shmClose();
                exit(EXIT_FAILURE);
            }
            MAX_VIDEO_WAIT = atribute;
            #ifdef DEBUG
                printf("MAX_VIDEO_WAIT = %d\n", MAX_VIDEO_WAIT);
            #endif
            break;
        case 5: // MAX_OTHERS_WAIT atribute
            if(atribute < 1) {
                error("MAX_OTHERS_WAIT < 1");
                shmClose();
                exit(EXIT_FAILURE);
            }
            MAX_OTHERS_WAIT = atribute;
            #ifdef DEBUG
                printf("MAX_OTHERS_WAIT = %d\n", MAX_OTHERS_WAIT);
            #endif
            break;
        default: // If the config file have more lines than it has suposed to have
            error("Too much lines in config file, must have 5 lines:\n<QUEUE_POS>\n<AUTH_SERVERS_MAX>\n<AUTH_PROC_TIME>\n<MAX_VIDEO_WAIT>\n<MAX_OTHERS_WAIT>");
            shmClose();
            exit(EXIT_FAILURE);
        }
        i++;
    }
    #ifdef DEBUG
        printf("File readed!\n");
    #endif
    fclose(f); // closes the config file
    free(buf);

    /* Creates the two child processes: Autorization Request Manager and the Monitor Engine writting a log after each creation */
    if((ME_PID = fork()) == 0) MonEng();
    MonEngCreated = true;
    writeLog("PROCESS MONITOR_ENGINE CREATED");
    if((ARM_PID = fork()) == 0) AutReqMan(ME_PID, QUEUE_POS, AUTH_SERVERS_MAX, AUTH_PROC_TIME, MAX_VIDEO_WAIT, MAX_OTHERS_WAIT);
    AutReqManCreated = true;
    writeLog("PROCESS AUTHORIZATION_REQUEST_MANAGER CREATED");

    /* Capture and handles the ^C (SIGINT) signal */
    signal(SIGINT, handle_sigint);

    /* Capture and handles the SIGQUIT signal */
    signal(SIGQUIT, handle_sigquit);

    /* Waits for his 2 childs to end */
    for(int j = 0; j < 2; j++) wait(NULL);

    /* Ends the System Manager */
    endSys();
    
    return 0;
}

/**
 * Closes and frees shared memory.
 */
void shmClose() {
    /* Unmaping shared memory */
    if (munmap(shm_ptr, shm_size) == -1) {
        error("SHM unmap");
        exit(EXIT_FAILURE);
    }
    /* Closing shared memory */
    if (close(shm_fd) == -1) {
        error("SHM close");
        exit(EXIT_FAILURE);
    }
    /* Unlinking shared memory */
    if (shm_unlink(SHM_PATH) == -1) {
        error("SHM unlink");
        exit(EXIT_FAILURE);
    }
}

/**
 * Kills child processes. ------------TODO LATER-----------
 */
void killProcess() {
    if(AutReqManCreated || MonEngCreated) kill(0, SIGQUIT);
    //  while(wait(NULL) != -1);
    if(AutReqManCreated) waitpid(ARM_PID);
    if(MonEngCreated) waitpid(ME_PID);
}

/**
 * Handles if the process catchs a sigquit signal.
 */
void handle_sigquit() {
    shmClose();
    exit(EXIT_SUCCESS);
}

/**
 * Handles if the process catchs a sigint signal.
 */
void handle_sigint() {
    /* Write log Signal received */
    writeLog("SIGNAL SIGINT RECEIVED");

    /* Write log Waiting for last task to finish */
    writeLog("5G_AUTH_PLATFORM SIMULATOR WAITING FOR LAST TASKS TO FINISH");
    killProcess();

    endSys();
}

/**
 * Ends the System Manager and it's child processes.
 */
void endSys() {
    #ifdef DEBUG
        printf("Ending program!\n");
    #endif
    
    shmClose();

    /* Ends the log file saving it and writing the last log saying the system will shut down */
    endLogFile();

    exit(EXIT_SUCCESS);
}