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

    /* Quando for mandar uma mensagem para um novo utilizador criar um named semaphore com o id do user na pasta dos named semaphores
     * Com isto podemos acrescentar ao semafero um recurso quando tiver la mais uma mensagem para o utilizador ler em primeiro da fila
     * Fazer uma queue com os semaferos onde vai sendo atualizada conforme a message queue 
     * NAO ACHO QUE A IDEIA FUNCIONE PENSANDO MELHOR ----- PERGUNTAR AO STOR COMO RESOLVER ESTA QUESTAO DA MESSAGE QUEUE E AS MESSAGENS SEREM LIDAS PELOS UTILIZADORES CORRETOS
        sem_t *sem = sem_open("/my_semaphore", O_CREAT, 0666, 1);
    */
    exit(EXIT_SUCCESS);
}