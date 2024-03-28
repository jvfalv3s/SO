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
#include "./LogFileManager/LogFileManager.h"
#include "./AutorizationReqManager/AutorizationReqManager.h"
#include "./MonitorEngine/MonitorEngine.h"

/* Comment this line to don't show debug messages */
#define DEBUG

void endSys();

const char* logFileName;
int shmid;
char* shmptr;

/**
 * Main Function.
 */
int main(int argc, char* argv[]) {
    /* Check if the arguments of execution are correct */
    if(argc != 2) {
        printf("Not enought arguments: ./SystemManager <config-file-name>");
        exit(0);
    }

    /* Create a new shared memory id and attach it */
    shmid = shmget(IPC_PRIVATE, 1024, IPC_CREAT | 0777);
    shmptr = (char*)shmat(shmid, NULL, 0);

    /* Creates a new log file and log the creation of the system manager process */
    logFileName = creatLogFile();
    writeLog(logFileName, "PROCESS SYSTEM_MANAGER CREATED");

    /* Initialization of the variables obtained from the config file */
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
    if((f = fopen(argv[0], "r")) == NULL) {
        error("Opening the config file");
    }
    while(fgets(buf, sizeof(buf), f)) {
        int atribute = atoi(buf);  // Transforming the strings in the file into integers
        /* The config file must follow an order so we use a counter to know which line we are */
        switch (i)
        {
        case 0: // QUEUE_POS atribute
            if(atribute < 0) error("QUEUE_POS < 0");
            QUEUE_POS = atribute;
            #ifdef DEBUG
                printf("QUEUE_POS = %d\n", QUEUE_POS);
            #endif
            break;
        case 1: // AUTH_SERVERS_MAX atribute
            if(atribute < 1) error("AUTH_SERVERS_MAX < 1");
            AUTH_SERVERS_MAX = atribute;
            #ifdef DEBUG
                printf("AUTH_SERVERS_MAX = %d\n", AUTH_SERVERS_MAX);
            #endif
            break;
        case 2: // AUTH_PROC_TIME atribute
            if(atribute < 0) error("AUTH_PROC_TIME < 0");
            AUTH_PROC_TIME = atribute;
            #ifdef DEBUG
                printf("AUTH_PROC_TIME = %d\n", AUTH_PROC_TIME);
            #endif
            break;
        case 3: // MAX_VIDEO_WAIT atribute
            if(atribute < 1) error("MAX_VIDEO_WAIT < 1");
            MAX_VIDEO_WAIT = atribute;
            #ifdef DEBUG
                printf("MAX_VIDEO_WAIT = %d\n", MAX_VIDEO_WAIT);
            #endif
            break;
        case 4: // MAX_OTHERS_WAIT atribute
            if(atribute < 1) error("MAX_OTHERS_WAIT < 1");
            MAX_OTHERS_WAIT = atribute;
            #ifdef DEBUG
                printf("MAX_OTHERS_WAIT = %d\n", MAX_OTHERS_WAIT);
            #endif
            break;
        default: // If the config file have more lines than it has suposed to have
            error("Too much lines in config file, must have 5 lines:\n<QUEUE_POS>\n<AUTH_SERVERS_MAX>\n<AUTH_PROC_TIME>\n<MAX_VIDEO_WAIT>\n<MAX_OTHERS_WAIT>");
            break;
        }
        i++;
    }
    #ifdef DEBUG
        printf("File readed!\n");
    #endif
    fclose(f); // closes the config file
    free(buf);

    /* Creates the two child processes: Autorization Request Manager and the Monitor Engine writting a log after each creation */
    pid_t ARM_PID, ME_PID;
    if((ARM_PID = fork()) == 0) AutReqMan(logFileName);
    writeLog(logFileName, "PROCESS AUTHORIZATION_REQUEST_MANAGER CREATED");
    if((ME_PID = fork()) == 0) MonEng(logFileName);
    writeLog(logFileName, "PROCESS AUTHORIZATION_REQUEST_MANAGER CREATED");

    /* Capture and handles the ^C (SIGINT) signal */
    signal(SIGINT, endSys);

    /* Waits for his 2 childs to end */
    for(int j = 0; j < 2; j++) wait(0);

    return 0;
}

/**
 * Ends the System Manager and it's child processes.
 * Not in final form.
 */
void endSys() {
    #ifdef DEBUG
        printf("Ending program!\n");
    #endif

    /* Write log Signal received */
    writeLog(logFileName, "SIGNAL SIGINT RECEIVED");

    /* Write log Waiting for last task to finish */
    writeLog(logFileName, "5G_AUTH_PLATFORM SIMULATOR WAITING FOR LAST TASKS TO FINISH");
    kill(0, SIGQUIT);
    for(int i = 0; i < 2; i++) wait();
    
    /* Detach and closes shared memory */
    shmdt(shmptr);
    shmctl(shmid, IPC_RMID, NULL);

    /* Ends the log file saving it and writing the last log saying the system will shut down */
    endLogFile(logFileName);

    exit(0);
}