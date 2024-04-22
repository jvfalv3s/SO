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
#include <unistd.h>
#include <sys/types.h>
#include <signal.h> 
#include <mqueue.h>

/* Path of the MESSAGE QUEUE */
#define MESSAGE_QUEUE_PATH "../tmp/FIFO/message_queue"
/* Path of the NAMED SEMAPHORES */
#define NAMED_SEMAPHORES_PATH "../tmp/FIFO/NAMED_SEMAPHORES/"

/* Initializing some usefull variables */
mqd_t mq;
struct mq_attr attr;
sem_t *sem

/**
 * Creates the Monitor Engine process.
 */
void MonEng(pid_t sys_pid) {
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10; // Maximum number of messages in the queue
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0;

    
    exit(EXIT_SUCCESS);
}