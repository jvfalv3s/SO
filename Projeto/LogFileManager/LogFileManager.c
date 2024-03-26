#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h> 
#include <unistd.h> 
#include "./LogFileManager.h"

sem_t mutex;
time_t t;
FILE* logFile;
struct tm tm;

const char* creatLogFile() {
    const char* logFileName;

    t = time(NULL);
    tm = *localtime(&t);
    sprintf(logFileName, "%02d-%02d-%d_%02d:%02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
    

    if((logFile = fopen(logFileName, "w")) == NULL) {
        error("Opening the config file");
    }
    fprintf(logFile, "5G_AUTH_PLATFORM SIMULATOR STARTING");
    fclose(logFile);

    sem_init(&mutex, 0, 1);

    return logFileName;
}

void writeLog(const char* logFileName, char* newLog) {
    sem_wait(&mutex);

    t = time(NULL);
    tm = *localtime(&t);

    if((logFile = fopen(logFileName, "a")) == NULL) {
        error("Opening the config file");
    }
    fprintf(logFile, "%02d:%02d:%02d %s", tm.tm_hour, tm.tm_min, tm.tm_sec, newLog);
    fclose(logFile);

    sem_post(&mutex);
}

void endLogFile(char* logFileName) {
    sem_wait(&mutex);

    if((logFile = fopen(logFileName, "a")) == NULL) {
        error("Opening the config file");
    }
    fprintf(logFile, "%02d:%02d:%02d 5G_AUTH_PLATFORM SIMULATOR CLOSING", tm.tm_hour, tm.tm_min, tm.tm_sec);
    fclose(logFile);

    sem_post(&mutex);
    
    sem_destroy(&mutex);
}