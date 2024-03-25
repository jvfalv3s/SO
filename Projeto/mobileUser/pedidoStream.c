#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define VIDEO_STREAMING_QUEUE_KEY 5678

// Estrutura para a mensagem na fila
struct video_request_message {
    long type; // Tipo da mensagem
    // Adicione campos adicionais conforme necessário para representar os dados do pedido
    // Por exemplo:
    // int user_id; // ID do usuário
    // int service_type; // Tipo de serviço (vídeo, música, etc.)
    // Outros campos relevantes
};

// Função para enviar um pedido de autorização para a fila de mensagens
void send_video_request(int queue_id, struct video_request_message *request) {
    // Enviar a mensagem para a fila
    if (msgsnd(queue_id, request, sizeof(struct video_request_message), 0) == -1) {
        perror("Erro ao enviar mensagem para a fila");
        exit(EXIT_FAILURE);
    }
}

int main() {
    // Criação ou obtenção da fila de mensagens
    int queue_id = msgget(VIDEO_STREAMING_QUEUE_KEY, IPC_CREAT | 0666);
    if (queue_id == -1) {
        perror("Erro ao criar ou obter a fila de mensagens");
        exit(EXIT_FAILURE);
    }

    // Criação de um pedido de autorização (substitua com a lógica real)
    struct video_request_message request;
    request.type = 1; // Tipo de mensagem (pode ser usado para distinguir entre diferentes tipos de pedidos)
    // Preencha os campos da mensagem conforme necessário
    // Por exemplo:
    // request.user_id = 123;
    // request.service_type = VIDEO;
    // Outros campos relevantes

    // Envio do pedido para a fila de mensagens
    send_video_request(queue_id, &request);

    // Encerramento da fila de mensagens (opcional)
    if (msgctl(queue_id, IPC_RMID, NULL) == -1) {
        perror("Erro ao encerrar a fila de mensagens");
        exit(EXIT_FAILURE);
    }

    return 0;
}
