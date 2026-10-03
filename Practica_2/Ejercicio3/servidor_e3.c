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

/* Estas dos constantes DEBEN coincidir con las del cliente.
   Si cambias una, cambia la otra o los programas se desincronizan. */
#define REPETICIONES   100
#define CALENTAMIENTO    5

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        exit(1);
    }

    /* Arreglo IDENTICO y en el mismo orden que el del cliente: asi ambos
       saben cuantos bytes corresponden a cada vuelta sin necesidad de
       negociar nada por la red. */
    const int tamanos[] = {10, 100, 1000, 10000, 100000, 1000000};
    const int num_tamanos = 6;

    int portno = atoi(argv[1]);

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
    printf("[SERVIDOR] Escuchando en el puerto %d...\n", portno);
    fflush(stdout);

    /* El accept() queda FUERA de los bucles: una sola conexion atiende
       los seis tamanios completos. */
    socklen_t clilen = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) error("Error en accept");

    printf("[SERVIDOR] Cliente conectado. Iniciando experimento.\n");
    printf("[SERVIDOR] %d repeticiones por tamanio (+%d de calentamiento).\n\n",
           REPETICIONES, CALENTAMIENTO);
    fflush(stdout);

    /* Buffer reservado UNA sola vez, del tamanio mayor */
    int max_tam = tamanos[num_tamanos - 1];
    char *buffer = (char *) malloc(max_tam);
    if (buffer == NULL) error("Error al reservar memoria");
    memset(buffer, 'A', max_tam);

    for (int i_tam = 0; i_tam < num_tamanos; i_tam++) {
        int tam_buffer = tamanos[i_tam];

        for (int iter = 0; iter < CALENTAMIENTO + REPETICIONES; iter++) {

            /* read() devuelve "lo que haya disponible", no necesariamente
               todo lo pedido. Con 10^5 y 10^6 bytes es practicamente seguro
               que haga falta insistir, asi que se lee en bucle hasta juntar
               los tam_buffer bytes. Sin esto el protocolo se desfasa en la
               vuelta siguiente. */
            int bytes_leidos = 0;
            while (bytes_leidos < tam_buffer) {
                int n = read(newsockfd, buffer + bytes_leidos,
                             tam_buffer - bytes_leidos);
                if (n < 0) error("Error en read");
                if (n == 0) {
                    fprintf(stderr,
                            "[SERVIDOR] El cliente cerro la conexion "
                            "(tamanio %d, iteracion %d)\n", tam_buffer, iter);
                    exit(1);
                }
                bytes_leidos += n;
            }

            /* Eco: una sola llamada a write() con los tam_buffer bytes
               recibidos, igual que el cliente. */
            write(newsockfd, buffer, tam_buffer);
        
        }

        printf("[SERVIDOR] Tamanio %7d bytes: %d intercambios completados.\n",
               tam_buffer, CALENTAMIENTO + REPETICIONES);
        fflush(stdout);
    }

    printf("\n[SERVIDOR] Experimento finalizado.\n");

    free(buffer);
    close(newsockfd);
    close(sockfd);
    return 0;
}
