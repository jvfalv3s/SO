/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

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

/**
 * Creates a new log file and returns his name.
 */
const char* creatLogFile() {
    const char* logFileName;

    /* Takes the actual time to use in the log file */
    t = time(NULL);
    tm = *localtime(&t);

    /* Define the log file name as dd-mm-yyyy_hh:mm:ss */
    sprintf(logFileName, "%02d-%02d-%d_%02d:%02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
    
    /* Creates a new file to write logs and writes the first log in it marking the initialization of the system */
    if((logFile = fopen(logFileName, "w")) == NULL) {
        error("Opening the config file");
    }
    fprintf(logFile, "%02d:%02d:%02d 5G_AUTH_PLATFORM SIMULATOR STARTING", tm.tm_hour, tm.tm_min, tm.tm_sec);
    fclose(logFile);

    /* Prints the same mensage in the console */
    printf("5G_AUTH_PLATFORM SIMULATOR STARTING");

    /* Initializes a new semaphore caled mutex to use in next times */
    sem_init(&mutex, 0, 1);

    /* Return the log file name to use in next logs */
    return logFileName;
}

/**
 * Writes a new log in the log file with the given name 'logFileName' and also prints the mensage in the console.
 */
void writeLog(const char* logFileName, char* newLog) {
    sem_wait(&mutex);

    /* Takes the actual time to use in the log file */
    t = time(NULL);
    tm = *localtime(&t);

    /* Opens the file to append a new log */
    if((logFile = fopen(logFileName, "a")) == NULL) {
        error("Opening the config file");
    }
    fprintf(logFile, "%02d:%02d:%02d %s", tm.tm_hour, tm.tm_min, tm.tm_sec, newLog);
    fclose(logFile);

    /* Prints the same mensage in the console */
    printf("%02d:%02d:%02d %s", tm.tm_hour, tm.tm_min, tm.tm_sec, newLog);

    sem_post(&mutex);
}

/**
 * Ends the log file writting a last log in it and in the console, it also destroys the mutex created.
 */
void endLogFile(char* logFileName) {
    sem_wait(&mutex);

    /* Takes the actual time to use in the log file */
    t = time(NULL);
    tm = *localtime(&t);

    /* Opens the file to append a new log */
    if((logFile = fopen(logFileName, "a")) == NULL) {
        error("Opening the config file");
    }
    fprintf(logFile, "%02d:%02d:%02d 5G_AUTH_PLATFORM SIMULATOR CLOSING", tm.tm_hour, tm.tm_min, tm.tm_sec);
    fclose(logFile);

    /* Prints the same mensage in the console */
    printf("%02d:%02d:%02d 5G_AUTH_PLATFORM SIMULATOR CLOSING", tm.tm_hour, tm.tm_min, tm.tm_sec);

    sem_post(&mutex);
    
    /* Destroys the mutex created to use the log file */
    sem_destroy(&mutex);
}

/**
 * Exists the program after an error printing the mensage in the screen and writting it in the log file.
 */
void error(char* error_message) {
    writeLog(logFileName, strcat("ERROR: ", strupr(error_message)));
    exit(0);
}