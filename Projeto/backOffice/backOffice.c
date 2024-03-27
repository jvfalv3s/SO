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

#define BACK_PIPE "back_pipe"
#define MESSAGE_QUEUE_KEY 1234

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
    while (1) {
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
