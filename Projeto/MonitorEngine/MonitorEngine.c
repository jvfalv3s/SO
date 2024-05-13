/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/
#include <sys/msg.h> 
#include <sys/types.h> // Add this line
#include <fcntl.h> // Add this line
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "./MonitorEngine.h"
#include "../ShmData.h"
#include "../HelpData.h"
#include "../MessageQueue.h"

#define MAX_STATISTICS_SIZE 256 // Tamanho máximo para as estatísticas

//funcao para processar os avisos
void process_alerts() {
    float plafond_percentage;
    while (1) {
        // sem_wait();
        for(int i = 0; i < shm_ptr->n_users; i++) {
            plafond_percentage = (float)shm_ptr->users[i].current_plafond / shm_ptr->users[i].max_plafond;
            message.mgg_type = shm_ptr->users[i].id;
            if (plafond_percentage >= 100) {
                // Plafond de 100% atingido para este usuário
                sprintf(message.msg_text, "Alerta: Seu plafond de dados foi totalmente utilizado.");
                msgsnd(mq_id, &message, sizeof(message), 0);
            } else if (plafond_percentage >= 90) {
                // Plafond de 90% ou mais atingido para este usuário
                sprintf(message.msg_text, "Alerta: Seu plafond de dados está quase totalmente utilizado.");
                msgsnd(mq_id, &message, sizeof(message), 0);
            } else if (plafond_percentage >= 80) {
                // Plafond de 80% ou mais atingido para este usuário
                sprintf(message.msg_text, "Alerta: Seu plafond de dados está quase 80%% utilizado.");
                msgsnd(mq_id, &message, sizeof(message), 0);
            }
        }
        // sem_wait();
    }
}

void endMonEng() {
    sem_close(sem);
    sem_unlink(MQ_SEM_PATH);
    msgctl(mq_id, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
}

void handle_sigquit() {
    endMonEng();
    exit(EXIT_SUCCESS);
}

void sendStatistics(const char* stats){
    printf("Estatísticas: %s\n", stats); // Imprime as estatísticas no terminal
}

void error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void MonEng() {
    /* Creating the message queue key */
    mq_key = ftok(MQ_KEY_PATH, MQ_KEY_ID);
    if(mq_key == -1) error("Creating message queue key");

    /* Opening the message queue for reading */
    mq_id = msgget(mq_key, IPC_CREAT | 0200);  // 0200 --> write-only permissions
    if(mq_id == -1) error("Getting message queue id");

    // Inicialização do semáforo nomeado
    sem = sem_open(MQ_SEM_PATH, O_CREAT | O_RDWR, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    signal(SIGQUIT, handle_sigquit);
    signal(SIGUSR1, process_alerts);

    while(true) {
        // send temp stats from back user
        // Obtenha suas estatísticas e as armazene na variável 'statistics'
        char statistics[MAX_STATISTICS_SIZE]; // Suponha que você tenha suas estatísticas em uma string

        // Formate as estatísticas em uma string
        sprintf(statistics, "Estatísticas: %s", "suas_estatisticas_aqui");

        // Envie as estatísticas para o terminal
        sendStatistics(statistics);


        sleep(30);
    }
    endMonEng();
}






/*
criar um semáforo nomeado para controlar o acesso à fila de mensagens e à memória compartilhada
inicializar o semáforo com o valor 1
criar um sinal para o monitoramento do consumo de dados
criar um sinal para o envio de alertas
criar um sinal para a compilação de estatísticas
criar um sinal para a finalização do processo
mandar ao log e ao terminal a mensagem de que o Monitor Engine foi criado e o plafond de cada usuário nos parametros esperados
no caso 80%, 90% e 100% do plafond
*/