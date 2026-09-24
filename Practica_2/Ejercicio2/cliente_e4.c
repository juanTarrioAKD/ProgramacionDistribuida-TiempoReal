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
#include <netdb.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <host_IP> <puerto> <tamano_buffer>\n", argv[0]);
        exit(1);
    }

    int portno = atoi(argv[2]);
    int tam_buffer = atoi(argv[3]); // Tamaño del búfer pasado por parámetro

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("Error abriendo socket");

    struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Error, no existe el host %s\n", argv[1]);
        exit(1);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("Error conectando");

    char *buffer = (char *) malloc(tam_buffer);
    if (buffer == NULL) error("Error al reservar memoria");
    memset(buffer, 'A', tam_buffer);

    struct timespec t0_write, t1_write;
    clock_gettime(CLOCK_MONOTONIC, &t0_write);

    // UNA ÚNICA LLAMADA A WRITE (SIN BUCLE WHILE)
    int bytes_enviados = write(sockfd, buffer, tam_buffer);

    clock_gettime(CLOCK_MONOTONIC, &t1_write);
    double tiempo_write = (t1_write.tv_sec - t0_write.tv_sec) + (t1_write.tv_nsec - t0_write.tv_nsec) / 1e9;

    // Recepción del ACK
    struct timespec t0_read, t1_read;
    clock_gettime(CLOCK_MONOTONIC, &t0_read);
    char ack;
    read(sockfd, &ack, 1);
    clock_gettime(CLOCK_MONOTONIC, &t1_read);
    double tiempo_read = (t1_read.tv_sec - t0_read.tv_sec) + (t1_read.tv_nsec - t0_read.tv_nsec) / 1e9;

    printf("\n=== RESULTADO CLIENTE (SIN WHILE) ===\n");
    printf("Tamaño de búfer solicitado a enviar: %d bytes\n", tam_buffer);
    printf("Bytes efectivamente entregados por write(): %d bytes\n", bytes_enviados);
    printf("Tiempo de write(): %.6f segundos\n", tiempo_write);
    printf("Tiempo de read() (ACK): %.6f segundos\n", tiempo_read);

    if (bytes_enviados < tam_buffer) {
        printf("[¡ATENCIÓN!] ESCRITURA PARCIAL: Solo se enviaron %d de %d bytes.\n", bytes_enviados, tam_buffer);
    } else {
        printf("[OK] Se entregaron todos los bytes a la red en una sola llamada.\n");
    }

    free(buffer);
    close(sockfd);
    return 0;
}
//fin