/**********************************************************
 *                         AUTHORES
 * 
 * --> Gonçalo José Carrajola Gaio           Nº: 2022224905
 * --> João Vitor Fraga Maia Alves           Nº: 2016122878
 **********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <stdbool.h>
//#include "backOfficeUser.h"
//#include "AutorizationReqManager.h"

#define BACK_PIPE "back_pipe"
#define MESSAGE_QUEUE_KEY 1234


pthread_t Sender_id, Receiver_id;

void* Sender(void *arg) {
    // Implemente aqui a lógica para o envio de comandos ao Autorization Request Manager
    return NULL;
}

void* Receiver(void *arg) {
    // Implemente aqui a lógica para o recebimento de estatísticas do Autorization Request Manager
    return NULL;
}

void handle_signal(int sig) {
    // Tratamento de sinal
    printf("Recebido sinal SIGINT. Encerrando o BackOffice User...\n");
    exitAutReqMan();
    exit(EXIT_SUCCESS);
}

void startBackOfficeUser() {
    // Registra o tratamento de sinal
    signal(SIGINT, handle_signal);

    // Abre o named pipe BACK_PIPE para escrita
    int back_pipe_fd = open(BACK_PIPE, O_WRONLY);
    if (back_pipe_fd == -1) {
        perror("Erro ao abrir o named pipe BACK_PIPE");
        exit(EXIT_FAILURE);
    }

    // Cria ou conecta à Message Queue
    int msgqid;
    if ((msgqid = msgget(MESSAGE_QUEUE_KEY, IPC_CREAT | 0666)) == -1) {
        perror("Erro ao criar/conectar a Message Queue");
        exit(EXIT_FAILURE);
    }

     // Crie as threads para envio e recebimento
    pthread_create(&Sender_id, NULL, Sender, NULL);
    pthread_create(&Receiver_id, NULL, Receiver, NULL);

    // Espere que as threads terminem
    pthread_join(Sender_id, NULL);
    pthread_join(Receiver_id, NULL);

    // Loop para receber estatísticas periódicas e solicitar estatísticas assíncronas
    while (!should_exit) { // Saia do loop se should_exit for verdadeiro
        // Implemente aqui a lógica para receber estatísticas periódicas e solicitar estatísticas assíncronas

        sleep(10); // Aguarda 10 segundos antes de continuar para simular o recebimento periódico de estatísticas
    }

    // Fecha o pipe BACK_PIPE
    close(back_pipe_fd);
}


// Função para receber estatísticas agregadas periodicamente
void receive_aggregated_statistics() {
    // Implemente a lógica para receber estatísticas agregadas da fila de mensagens
}

// Função para solicitar estatísticas de forma assíncrona ao Authorization Requests Manager
void request_statistics(int back_pipe_fd) {
    char request[] = "STATS"; // Exemplo de solicitação de estatísticas
    write(back_pipe_fd, request, strlen(request) + 1);
}

int main() {
    // Criar o named pipe para comunicação com o Authorization Requests Manager
    if (mkfifo(BACK_PIPE, 0666) == -1) {
        if (errno != EEXIST) {
            perror("Erro ao criar o named pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Abrir o named pipe para escrita
    int back_pipe_fd = open(BACK_PIPE, O_WRONLY);
    if (back_pipe_fd == -1) {
        perror("Erro ao abrir o named pipe para escrita");
        exit(EXIT_FAILURE);
    }

    // Loop principal do processo BackOffice User
    while (!should_exit) {
        // Receber estatísticas agregadas periodicamente
        receive_aggregated_statistics();

        // Solicitar estatísticas de forma assíncrona ao Authorization Requests Manager
        request_statistics(back_pipe_fd);

        // Esperar um intervalo de tempo antes de receber novamente (por exemplo, 10 segundos)
        sleep(10);
    }

    // Fechar o named pipe
    close(back_pipe_fd);
    // Remover o named pipe
    unlink(BACK_PIPE);

    return 0;
}
