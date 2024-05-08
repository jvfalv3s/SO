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
#include <mqueue.h>
#include <semaphore.h>
#include "MonitorEngine.h"

#define MAX_SIZE 1024
#define MESSAGE_QUEUE_PATH "../tmp/FIFO/message_queue"
#define NAMED_SEMAPHORES_PATH "../tmp/FIFO/NAMED_SEMAPHORES/"

mqd_t mq;
struct mq_attr attr;
sem_t *sem;
pid_t SYS_PID;

void MonEng(pid_t SYS_PID) {
    // Inicialização da fila de mensagens
    mq = mq_open(MESSAGE_QUEUE_PATH, O_CREAT | O_RDWR, 0666, &attr);
    if (mq == -1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    // Inicialização do semáforo nomeado
    sem = sem_open(NAMED_SEMAPHORES_PATH, O_CREAT | O_RDWR, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Monitoramento contínuo do consumo de dados
        // Código para ler o consumo de dados de cada usuário e comparar com os limites

        // Geração de alertas
        if (consumo_critico) {
            // Envio de mensagem de alerta para a fila de mensagens
            char msg[MAX_SIZE];
            sprintf(msg, "Usuário %d: consumo de dados crítico", id_usuario);
            mq_send(mq, msg, strlen(msg) + 1, 0);
        }

        // Compilação de estatísticas
        // Código para compilar estatísticas periódicas
    }
}

int main() {
    MonEng(SYS_PID);
    return 0;
}
