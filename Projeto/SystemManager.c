#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "./LogFileManager/LogFileManager.h"
#include <sys/types.h>

/* Comment this line to don't show debug mensages */
#define DEBUG

#define CONFIG_FILE "./ConfigFile.txt"

void error(char* error_message);
void AutReqMan();
void MonEng();

const char* logFileName;

int main(int argc, char* argv[]) {
    logFileName = creatLogFile();
    writeLog(logFileName, "PROCESS SYSTEM_MANAGER CREATED");

    int QUEUE_POS;
    int AUTH_SERVERS_MAX;
    int AUTH_PROC_TIME;
    int MAX_VIDEO_WAIT;
    int MAX_OTHERS_WAIT;

    FILE* f;
    char* buf = (char*) malloc(sizeof(char)*100);
    int i = 0;

    #ifdef DEBUG
        printf("Start reading file...\n");
    #endif
    if((f = fopen(CONFIG_FILE, "r")) == NULL) {
        error("Opening the config file");
    }
    while(fgets(buf, sizeof(buf), f)) {
        int atribute = atoi(buf);
        switch (i)
        {
        case 0:
            if(atribute < 0) error("QUEUE_POS < 0");
            QUEUE_POS = atribute;
            #ifdef DEBUG
                printf("QUEUE_POS = %d\n", QUEUE_POS);
            #endif
            break;
        case 1:
            if(atribute < 1) error("AUTH_SERVERS_MAX < 1");
            AUTH_SERVERS_MAX = atribute;
            #ifdef DEBUG
                printf("AUTH_SERVERS_MAX = %d\n", AUTH_SERVERS_MAX);
            #endif
            break;
        case 2:
            if(atribute < 0) error("AUTH_PROC_TIME < 0");
            AUTH_PROC_TIME = atribute;
            #ifdef DEBUG
                printf("AUTH_PROC_TIME = %d\n", AUTH_PROC_TIME);
            #endif
            break;
        case 3:
            if(atribute < 1) error("MAX_VIDEO_WAIT < 1");
            MAX_VIDEO_WAIT = atribute;
            #ifdef DEBUG
                printf("MAX_VIDEO_WAIT = %d\n", MAX_VIDEO_WAIT);
            #endif
            break;
        case 4:
            if(atribute < 1) error("MAX_OTHERS_WAIT < 1");
            MAX_OTHERS_WAIT = atribute;
            #ifdef DEBUG
                printf("MAX_OTHERS_WAIT = %d\n", MAX_OTHERS_WAIT);
            #endif
            break;
        default:
            error("Too much lines in config file, must have 5 lines:\n<QUEUE_POS>\n<AUTH_SERVERS_MAX>\n<AUTH_PROC_TIME>\n<MAX_VIDEO_WAIT>\n<MAX_OTHERS_WAIT>");
            break;
        }
        i++;
    }
    #ifdef DEBUG
        printf("File readed!\n");
    #endif
    fclose(f);
    free(buf);


    pid_t ARM_PID, ME_PID;
    if(ARM_PID = fork() == 0) AutReqMan();
    writeLog(logFileName, "PROCESS AUTHORIZATION_REQUEST_MANAGER CREATED");
    if(ME_PID = fork() == 0) MonEng();
    writeLog(logFileName, "PROCESS AUTHORIZATION_REQUEST_MANAGER CREATED");

    for(int j = 0; j < 2; j++) wait();

    return 0;
}

void error(char* error_message) {
    printf("%s\n", error_message);
    exit(0);
}

void endSys() {
    endLogFile(logFileName);
    exit(0);
}