#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]); // Corregido: argv[0]
        exit(1);
    }

    int portno = atoi(argv[1]); // Corregido: argv[1]
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("Error al abrir socket");

    struct sockaddr_in serv_addr, cli_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("Error en bind");

    listen(sockfd, 5);
    printf("[SERVIDOR] Escuchando en el puerto %d...\n", portno);

    socklen_t clilen = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) error("Error en accept");

    // Procesamos las 6 pruebas de tamaño (10^1 a 10^6)
    int tamanios[] = {10, 100, 1000, 10000, 100000, 1000000};
    
    for (int i = 0; i < 6; i++) {
        int tamano_esperado = tamanios[i];
        char *buffer = (char *) malloc(tamano_esperado);
        if (buffer == NULL) error("Error al reservar memoria en servidor");

        int total_recibido = 0;
        // Bucle para controlar LECTURAS PARCIALES
        while (total_recibido < tamano_esperado) {
            int n = read(newsockfd, buffer + total_recibido, tamano_esperado - total_recibido);
            if (n < 0) error("Error leyendo del socket");
            if (n == 0) break; // El cliente cerró la conexión inesperadamente
            total_recibido += n;
        }

        // VERIFICACIÓN sin imprimir el buffer completo
        int datos_correctos = 1;
        for (int j = 0; j < total_recibido; j++) {
            if (buffer[j] != 'A') { // Esperamos que todos los bytes sean el carácter 'A'
                datos_correctos = 0;
                break;
            }
        }

        if (total_recibido == tamano_esperado && datos_correctos) {
            printf("[SERVIDOR] [OK] Recibidos correctamente %d bytes (10^%d).\n", total_recibido, i + 1);
        } else {
            printf("[SERVIDOR] [ERROR] Falla en prueba de %d bytes. Recibidos: %d\n", tamano_esperado, total_recibido);
        }

        free(buffer);
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}