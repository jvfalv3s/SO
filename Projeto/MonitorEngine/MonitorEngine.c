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
#include <signal.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <semaphore.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include "ShmData.h"
#include "MonitorEngine.h"

#define MAX_SIZE 1024
#define NAMED_SEMAPHORES_PATH "../tmp/FIFO/NAMED_SEMAPHORES/"
#define MAX_CHAR_MESSAGE_AMMOUNT 100
/* ftok arguments to create the message queue key */
#define MQ_KEY_PATH "/message_queue"
#define MQ_KEY_ID 'a'

int mq_id;                               // Message queue id
struct mq_message message;               // Message from message queue
char* mq_named_sem_path;                 // Path to message queue named semaphore
sem_t *sem;
pid_t SYS_PID;

/* Message from message queue struct */
typedef struct mq_message {
    long mgg_type;
    char msg_text [MAX_CHAR_MESSAGE_AMMOUNT];
};

user users[MAX_USERS_SHM]; // Array de usuarios

//criar threads para monitorar o consumo de dados e gerar alertas
//declaracao das threads 
pthread_t thread_monitor, thread_alerts, thread_plafond, thread_stats, thread_exit;
PTHREAD_MUTEX_INITIALIZER; // Mutex for users array

//mutex para proteger a estrutura de dados dos usuarios
pthread_mutex_t users_mutex = PTHREAD_MUTEX_INITIALIZER; 

//funcao para verificar a atualizacao dos planfods dos usuarios

void* updatePlafond(void* arg) {
    while (1) {
        //atualiza o plafond de cada usuario
        pthread_mutex_lock(&users_mutex);
        for (int i = 0; i < MAX_USERS_SHM; i++) {
            //verifica o plafond atual do usuario
            users[i].plafond = users[i].current_plafond / users[i].max_plafond;  
        }
        phtread_mutex_unlock(&users_mutex);

        //aguardar um intervalo de tempo
        sleep(5);
    }
    return NULL;
}

//funcao para monitorar o System manager e os usuarios
void* monitor_system(void* arg){
    while(1){
        //monitorar o system managere os usuarios
        //codigo para monitorar o system manager e os usuarios

        //verificar se o plafond de algum usuario atingiu os limites
        pthread_mutex_lock(&users_mutex);
        for(int i =0; i < MAX_USERS_SHM; i++){
            if(users[i].plafond >= plafondThreshold){
                //envia um sinal para a thread de alertas
                pthread_kill(thread_alerts, SIGUSR1);
                break;
            }
        }
        pthread_mutex_unlock(&users_mutex);

        //aguardar um intervalo de tempo
        sleep(1); //intervalo de 1 segundo
    }
    return NULL;
}
//funcao para processar os avisos
void* process_alerts(void* arg){
    while(1){
        //aguarda um sinal para processar os avisos
        signal(SIGUSR1, plafondThreshold);
    }
    return NULL;
}

//funcao para processar os avisos
void* monitor_system(void* arg) {
    while (1) {
        // Monitora o System Manager e os usuários
        // Código para monitorar o System Manager e os usuários

        // Verifica se o plafond de algum usuário atingiu os limites
        pthread_mutex_lock(&users_mutex);
        for (int i = 0; i < MAX_USERS_SHM; i++) {
            float plafond_percentage = (float)users[i].plafond / user.max_plafond;
            if (plafond_percentage >= 100) {
                // Plafond de 100% atingido para este usuário
                message.msg_type = 1;
                sprintf(message.msg_text, "Alerta: Seu plafond de dados foi totalmente utilizado.");
                msgsnd(mq_id, &message, sizeof(message), 0);
            } else if (plafond_percentage >= 90) {
                // Plafond de 90% ou mais atingido para este usuário
                message.msg_type = 1;
                sprintf(message.msg_text, "Alerta: Seu plafond de dados está quase totalmente utilizado.");
                msgsnd(mq_id, &message, sizeof(message), 0);
            } else if (plafond_percentage >= 80) {
                // Plafond de 80% ou mais atingido para este usuário
                message.msg_type = 1;
                sprintf(message.msg_text, "Alerta: Seu plafond de dados está quase 80%% utilizado.");
                msgsnd(mq_id, &message, sizeof(message), 0);
            }
        }
        pthread_mutex_unlock(&users_mutex);

        // Aguarda um intervalo de tempo
        sleep(1); // Intervalo de 1 segundo
    }
    return NULL;
}

int create_threads(){
    // Inicializar usuários (apenas para simulação)
    for (int i = 0; i < MAX_USERS_SHM; i++) {
        users[i].id = i;
        users[i].plafond = rand() % 101; // Plafond inicial aleatório
    }

    // Criar threads
    pthread_create(&thread_monitor, NULL, monitor_system, NULL);
    pthread_create(&thread_alerts, NULL, process_alerts , NULL);
    pthread_create(&thread_plafond, NULL, updatePlafond, NULL);

    // Aguardar as threads terminarem
    pthread_join(thread_monitor, NULL);
    pthread_join(thread_alerts, NULL);
    pthread_join(thread_plafond, NULL);

    return 0;
}

void MonEng(pid_t SYS_PID, int consumo_critico,int id_usuario) {
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

    signal(SIGUSR1, plafondThreshold);

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