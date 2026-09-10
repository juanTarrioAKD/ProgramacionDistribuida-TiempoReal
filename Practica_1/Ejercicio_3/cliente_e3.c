#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <host> <puerto>\n", argv[0]);
        exit(1);
    }

    int portno = atoi(argv[2]);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("Error abriendo socket");

    struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no existe el host %s\n", argv[4]);
        exit(0);
    }

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("Error conectando");

    // Tamaños a probar: 10^1 a 10^6 bytes
    int tamanios[] = {10, 100, 1000, 10000, 100000, 1000000};

    for (int i = 0; i < 6; i++) {
        int tamano = tamanios[i];
        
        // Asignación directa en memoria e inicialización con datos patron
        char *buffer = (char *) malloc(tamano);
        if (buffer == NULL) error("Error al reservar memoria");
        memset(buffer, 'A', tamano); // Llenamos el buffer con el carácter 'A'

        int total_enviado = 0;
        // Bucle para controlar ESCRITURAS PARCIALES
        while (total_enviado < tamano) {
            int n = write(sockfd, buffer + total_enviado, tamano - total_enviado);
            if (n < 0) error("Error escribiendo en socket");
            total_enviado += n;
        }

        printf("[CLIENTE] Enviados exitosamente %d bytes (10^%d).\n", total_enviado, i + 1);
        free(buffer);
        usleep(100000); // Pausa de 100ms entre envíos
    }

    close(sockfd);
    return 0;
}