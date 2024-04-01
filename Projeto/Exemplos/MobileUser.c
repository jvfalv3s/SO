#include "MobileUser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define USER_PIPE "USER_PIPE"
#define MESSAGE_QUEUE_KEY 1234

pthread_t Sender_id, Receiver_id;

void* Sender(void *arg) {
    // Implemente aqui a lógica para o envio de mensagens para o Autorization Request Manager
    return NULL;
}

void* Receiver(void *arg) {
    // Implemente aqui a lógica para o recebimento de alertas do Autorization Request Manager
    return NULL;
}

void handle_signal(int sig) {
    // Tratamento de sinal
    printf("Recebido sinal SIGINT. Encerrando o Mobile User...\n");
    exitAutReqMan();
    exit(EXIT_SUCCESS);
}

void startMobileUser(int argc, char *argv[]) {
    if (argc < 7) {
        fprintf(stderr, "Uso: %s <plafond inicial> <número máximo de pedidos> <intervalo VIDEO> <intervalo MUSIC> <intervalo SOCIAL> <dados a reservar>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Registra o tratamento de sinal
    signal(SIGINT, handle_signal);

    // Abre o named pipe USER_PIPE para escrita
    int user_pipe_fd = open(USER_PIPE, O_WRONLY);
    if (user_pipe_fd == -1) {
        perror("Erro ao abrir o named pipe USER_PIPE");
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

    // Envia mensagem de registo inicial
    dprintf(user_pipe_fd, "%d#%s\n", getpid(), argv[1]);

    // Cria o processo Autorization Request Manager
    AutReqMan("logfile.txt");

    // Fecha o pipe USER_PIPE
    close(user_pipe_fd);
}
