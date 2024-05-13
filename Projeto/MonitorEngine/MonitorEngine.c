/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/
#include <sys/msg.h> 
#include <semaphore.h>
#include <time.h>
#include "./MonitorEngine.h"
#include "../ShmData.h"
#include "../HelpData.h"

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