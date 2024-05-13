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
#include <sys/types.h>
#include <mqueue.h>
#include <semaphore.h>

#define MAX_SIZE 1024
#define MESSAGE_QUEUE_PATH "../tmp/FIFO/message_queue"
#define NAMED_SEMAPHORES_PATH "../tmp/FIFO/NAMED_SEMAPHORES/"

extern mqd_t mq;
extern sem_t *sem;
extern pid_t SYS_PID;

void MonEng();
void plafondThreshold(int sinal);

#endif

