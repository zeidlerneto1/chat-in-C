#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

char nome_usuario[50];

void* receber_mensagens(void* socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes <= 0) {
            printf("Conexão encerrada.\n");
            break;
        }
        printf("\n%s\n> ", buffer); 
        fflush(stdout);
    }
    return NULL;
}

void servidor(int porta) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(porta);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 1);

    printf("Aguardando conexão na porta %d...\n", porta);
    new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    printf("Cliente conectado!\n");

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receber_mensagens, &new_socket);

    while (1) {
        printf("> ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        char mensagem[BUFFER_SIZE];
        snprintf(mensagem, BUFFER_SIZE, "%s: %s", nome_usuario, buffer);
        send(new_socket, mensagem, strlen(mensagem), 0);
    }

    close(new_socket);
    close(server_fd);
}

void cliente(const char* ip, int porta) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(porta);
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erro na conexão");
        return;
    }

    printf("Conectado ao servidor %s:%d\n", ip, porta);

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receber_mensagens, &sock);

    while (1) {
        printf("> ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        char mensagem[BUFFER_SIZE];
        snprintf(mensagem, BUFFER_SIZE, "%s: %s", nome_usuario, buffer);
        send(sock, mensagem, strlen(mensagem), 0);
    }

    close(sock);
}

int main() {
    int opcao, porta;
    char ip[50];

    printf("Digite seu nome de usuário: ");
    fgets(nome_usuario, sizeof(nome_usuario), stdin);
    nome_usuario[strcspn(nome_usuario, "\n")] = 0;

    printf("Escolha uma opção:\n");
    printf("1. Rodar como servidor\n");
    printf("2. Rodar como cliente\n");
    printf("Opção: ");
    scanf("%d", &opcao);
    getchar();
  
    printf("Informe a porta: ");
    scanf("%d", &porta);
    getchar();

    if (opcao == 1) {
        printf("Aviso: para acesso externo, faça port forwarding na porta %d.\n", porta);
        servidor(porta);
    } else if (opcao == 2) {
        printf("Informe o IP do servidor (ex: 127.0.0.1): ");
        fgets(ip, sizeof(ip), stdin);
        ip[strcspn(ip, "\n")] = 0;
        cliente(ip, porta);
    } else {
        printf("Opção inválida.\n");
    }

    return 0;
}
