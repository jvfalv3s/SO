/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#ifndef HELPDATA_H
#define HELPDATA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int MOBILE_USERS;
int QUEUE_POS;
int AUTH_SERVERS_MAX;
int AUTH_PROC_TIME;
int MAX_VIDEO_WAIT;
int MAX_OTHERS_WAIT;
bool AutReqManCreated = false, MonEngCreated = false;

pid_t SYS_PID, ARM_PID, ME_PID;

#endif