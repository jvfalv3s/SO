/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/


/*a analise desse processo serve para determinar quando o plafond de cada Mobile User atinge de 80 a 100%
do plafond inicial. Como resultado gera-se alertas para os respectivos MU atravea da messa queue. Gera tb 
estatisticas periodicas (30s) para o Backoffice  User */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <semaphore.h>

#define MAX_SIZE 1024
#define NAMED_SEMAPHORES_PATH "../tmp/FIFO/NAMED_SEMAPHORES/"
#define MAX_CHAR_MESSAGE_AMMOUNT 100
/* ftok arguments to create the message queue key */
#define MQ_KEY_PATH "/message_queue"
#define MQ_KEY_ID 'a'


//cria as funções signal para 80, 90 e 100%

//essas funcões tem que implementar uma mensagem pro log (wirtelog o que eu qurioser)

//vou ter que enviar a mesma mensagem pro message queue.

//ligar do ARM pra ca, usando o PID MON EN -> pesquisar como se faz isso. slides (8/9)

/* Message from message queue struct */
typedef struct mq_message {
    long mgg_type;
    char msg_text [MAX_CHAR_MESSAGE_AMMOUNT];
};

int mq_id;                               // Message queue id
struct mq_message message;               // Message from message queue
char* mq_named_sem_path;                 // Path to message queue named semaphore
sem_t *sem;
pid_t SYS_PID;

void MonEng(int consumo_critico,int id_usuario) {
    /* Creating the message queue key */
    int mq_key = ftok(MQ_KEY_PATH, MQ_KEY_ID);
    if(mq_key == -1) error("Creating message queue key");

    /* Opening the message queue for reading */
    mq_id = msgget(mq_key, IPC_CREAT | 0200);  // 0200 --> write-only permissions
    if(mq_id == -1) error("Getting message queue id");

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
            mq_send(mq_id, msg, strlen(msg) + 1, 0); // Replace 'mq' with 'mq_id'
        }

        // Compilação de estatísticas
        // Código para compilar estatísticas periódicas
    }
}


caso chegou no patamar, manda um kill(pid mon, sinal qlqr)

signal(3, funcao ola);