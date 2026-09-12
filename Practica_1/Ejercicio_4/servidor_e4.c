#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <puerto> <tamano_buffer>\n", argv[0]);
        exit(1);
    }

    int portno = atoi(argv[1]);
    int tam_buffer = atoi(argv[2]); // Tamaño del búfer pasado por parámetro

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("Error al abrir socket");

    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct sockaddr_in serv_addr, cli_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("Error en bind");

    listen(sockfd, 5);
    printf("[SERVIDOR - VM A] Escuchando en el puerto %d (Buffer configurado: %d bytes)...\n", portno, tam_buffer);

    socklen_t clilen = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) error("Error en accept");

    char *buffer = (char *) malloc(tam_buffer);
    if (buffer == NULL) error("Error al reservar memoria");

    struct timespec t0_read, t1_read;
    clock_gettime(CLOCK_MONOTONIC, &t0_read);

    // UNA ÚNICA LLAMADA A READ (SIN BUCLE WHILE)
    int bytes_leidos = read(newsockfd, buffer, tam_buffer);

    clock_gettime(CLOCK_MONOTONIC, &t1_read);
    double tiempo_read = (t1_read.tv_sec - t0_read.tv_sec) + (t1_read.tv_nsec - t0_read.tv_nsec) / 1e9;

    printf("\n=== RESULTADO SERVIDOR (SIN WHILE) ===\n");
    printf("Tamaño de búfer esperado: %d bytes\n", tam_buffer);
    printf("Bytes efectivamente leídos en 1 sola llamada a read(): %d bytes\n", bytes_leidos);
    printf("Tiempo de read(): %.6f segundos\n", tiempo_read);

    if (bytes_leidos < tam_buffer) {
        printf("[¡ATENCIÓN!] SE PRODUJO LECTURA PARCIAL: Se dejaron de leer %d bytes (%d recibidos de %d esperados).\n", 
               tam_buffer - bytes_leidos, bytes_leidos, tam_buffer);
    } else {
        printf("[OK] Se leyó la totalidad del búfer en una sola llamada.\n");
    }

    // Confirmación (ACK) enviada al cliente
    write(newsockfd, "K", 1);

    free(buffer);
    close(newsockfd);
    close(sockfd);
    return 0;
}