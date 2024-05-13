/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/
#include <sys/msg.h> 
#include <semaphore.h>
#include <time.h>
#include "MonitorEngine.h"
#include "ShmData.h"
#include "HelpData.h"

#define MAX_SIZE 1024
#define MQ_SEM_PATH "mq_sem"
#define MAX_CHAR_MESSAGE_AMMOUNT 100
/* ftok arguments to create the message queue key */
#define MQ_KEY_PATH "/message_queue"
#define MQ_KEY_ID 'a'

int mq_id;                               // Message queue id
struct mq_message message;               // Message from message queue
char* mq_named_sem_path;                 // Path to message queue named semaphore
sem_t *sem;

/* Message from message queue struct */
typedef struct mq_message {
    long mgg_type;
    char msg_text [MAX_CHAR_MESSAGE_AMMOUNT];
};

void MonEng() {
    /* Creating the message queue key */
    int mq_key = ftok(MQ_KEY_PATH, MQ_KEY_ID);
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
        // send temp stats to back user

        sleep(30);
    }
    endMonEng();
}

//funcao para processar os avisos
void process_alerts() {
    float plafond_percentage;
    while (1) {
        // sem_wait();
        for(int i = 0; i < shm_ptr->n_users; i++) {
            plafond_percentage = (float)shm_ptr->users[i].plafond / user.max_plafond;
            message.msg_type = shm_ptr->users[i].id;
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