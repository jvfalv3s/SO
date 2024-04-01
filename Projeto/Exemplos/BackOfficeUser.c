#include "BackOfficeUser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define BACK_PIPE "BACK_PIPE"
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
    while (1) {
        // Implemente aqui a lógica para receber estatísticas periódicas e solicitar estatísticas assíncronas

        sleep(10); // Aguarda 10 segundos antes de continuar para simular o recebimento periódico de estatísticas
    }

    // Fecha o pipe BACK_PIPE
    close(back_pipe_fd);
}
