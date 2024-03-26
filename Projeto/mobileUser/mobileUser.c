#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define USER_PIPE "user_pipe"
#define VIDEO_STREAMING_QUEUE_KEY 5678
#define MESSAGE_QUEUE_KEY 1234

// Estrutura para as mensagens na fila
struct message_buffer {
    long message_type;
    char message_text[100];
};

// Estrutura para a mensagem na fila de vídeo
struct video_request_message {
    long type; // Tipo da mensagem
    // Adicione campos adicionais conforme necessário para representar os dados do pedido
    // Por exemplo:
    // int user_id; // ID do usuário
    // int service_type; // Tipo de serviço (vídeo, música, etc.)
    // Outros campos relevantes
};

// Etrutura pra a mensagem na fila de música

struct music_request_message {
    long type; // Tipo da mensagem
    // Adicione campos adicionais conforme necessário para representar os dados do pedido
    // Por exemplo:
    // int user_id; // ID do usuário
    // int service_type; // Tipo de serviço (vídeo, música, etc.)
    // Outros campos relevantes
};

// Estrutura para a mensagem na fila de rede social

struct social_request_message {
    long type; // Tipo da mensagem
    // Adicione campos adicionais conforme necessário para representar os dados do pedido
    // Por exemplo:
    // int user_id; // ID do usuário
    // int service_type; // Tipo de serviço (vídeo, música, etc.)
    // Outros campos relevantes
};

// Função para enviar um pedido de serviço
void send_data_request(int user_pipe_fd, int video_queue_id, int initial_balance) {
    // Enviar mensagem de registo inicial para o Authorization Requests Manager
    char registration_message[100];
    sprintf(registration_message, "%d#%d", getpid(), initial_balance);
    write(user_pipe_fd, registration_message, strlen(registration_message) + 1);

    // Exemplo de pedido de serviço (música)
    char request2[] = "MUSIC"; // Exemplo de pedido de serviço (música)
    write(user_pipe_fd, request2, strlen(request2) + 1);

    // Exemplo de pedido de serviço (rede social)
    char request3[] = "SOCIAL"; // Exemplo de pedido de serviço (rede social)
    write(user_pipe_fd, request3, strlen(request3) + 1);

    // Criação de um pedido de autorização para a fila de mensagens de vídeo
    struct video_request_message video_request;
    video_request.type = 1; // Tipo de mensagem (pode ser usado para distinguir entre diferentes tipos de pedidos)
    // Preencha os campos da mensagem conforme necessário
    // Por exemplo:
    // video_request.user_id = 123;
    // video_request.service_type = VIDEO;
    // Outros campos relevantes

    // Envio do pedido para a fila de mensagens de vídeo
    if (msgsnd(video_queue_id, &video_request, sizeof(struct video_request_message), 0) == -1) {
        perror("Erro ao enviar pedido de vídeo para a fila");
        exit(EXIT_FAILURE);
    }
}

// Função para receber alertas
void receive_alerts(int message_queue_id) {
    struct message_buffer message;
    
    // Receber alertas da fila de mensagens
    if (msgrcv(message_queue_id, &message, sizeof(message), 1, IPC_NOWAIT) != -1) {
        // Exibir alerta
        printf("Alerta: %s\n", message.message_text);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Uso: %s <plafond_inicial> <n_pedidos_autorizacao> <intervalo_VIDEO> <intervalo_MUSIC> <intervalo_SOCIAL> <dados_reservar>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Extrair argumentos da linha de comando
    int initial_balance = atoi(argv[1]);
    int num_authorization_requests = atoi(argv[2]);
    int interval_VIDEO = atoi(argv[3]);
    int interval_MUSIC = atoi(argv[4]);
    int interval_SOCIAL = atoi(argv[5]);
    int data_to_reserve = atoi(argv[6]);

    // Criar o named pipe para comunicação com o Authorization Requests Manager
    if (mkfifo(USER_PIPE, 0666) == -1) {
        if (errno != EEXIST) {
            perror("Erro ao criar o named pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Abrir o named pipe para escrita
    int user_pipe_fd = open(USER_PIPE, O_WRONLY);
    if (user_pipe_fd == -1) {
        perror("Erro ao abrir o named pipe para escrita");
        exit(EXIT_FAILURE);
    }

    // Criar ou obter a fila de mensagens de vídeo
    int video_queue_id = msgget(VIDEO_STREAMING_QUEUE_KEY, IPC_CREAT | 0666);
    if (video_queue_id == -1) {
        perror("Erro ao criar ou obter a fila de mensagens de vídeo");
        exit(EXIT_FAILURE);
    }

    // Criar ou obter a fila de mensagens
    int message_queue_id = msgget(MESSAGE_QUEUE_KEY, IPC_CREAT | 0666);
    if (message_queue_id == -1) {
        perror("Erro ao criar ou obter a fila de mensagens");
        exit(EXIT_FAILURE);
    }

    // Loop principal do processo Mobile User
    int i;
    for (i = 0; i < num_authorization_requests; ++i) {
        // Gerar e enviar pedidos de serviço
        send_data_request(user_pipe_fd, video_queue_id, initial_balance);

        // Receber alertas relacionados com os consumos dos vários serviços
        receive_alerts(message_queue_id);

        // Esperar um intervalo de tempo antes de enviar outro pedido (por exemplo, 1 segundo)
        sleep(1);
    }

    // Fechar o named pipe
    close(user_pipe_fd);
    // Remover o named pipe
    unlink(USER_PIPE);

    // Remover a fila de mensagens de vídeo
    if (msgctl(video_queue_id, IPC_RMID, NULL) == -1) {
        perror("Erro ao encerrar a fila de mensagens de vídeo");
        exit(EXIT_FAILURE);
    }

    // Remover a fila de mensagens
    if (msgctl(message_queue_id, IPC_RMID, NULL) == -1) {
        perror("Erro ao encerrar a fila de mensagens");
        exit(EXIT_FAILURE);
    }

    return 0;
}
