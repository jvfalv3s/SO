/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#ifndef MONITOR_ENGINE_H
#define MONITOR_ENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <mqueue.h>
#include <semaphore.h>

/**
 *  This function is responsible for the Monitor Engine
 */
void MonEng();

/**
 *    Process alerts when certain signals are received
 */
void process_alerts();

/**
 *   Sends statistics to the back user and to the log file
 */
void sendStatistics();

/**
 *   Handle errors in the Monitor Engine
 */
void MonEngError(char* error_message);

/**
 *   End the Monitor Engine
 */
void endMonEng();

#endif

