#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define REPETICIONES   100   /* mediciones que se contabilizan por tamanio */
#define CALENTAMIENTO    5   /* vueltas previas que se ejecutan y se descartan */
#define MAX_INTENTOS    30   /* reintentos de connect(), uno por segundo */

void error(const char *msg) {
    perror(msg);
    exit(1);
}

/* Diferencia b - a en nanosegundos, con enteros.
   El termino de tv_nsec puede dar negativo cuando el segundo entero avanzo:
   eso es correcto, compensa exactamente ese segundo de mas. */
static long long ns_entre(struct timespec *a, struct timespec *b) {
    return (long long)(b->tv_sec  - a->tv_sec) * 1000000000LL
         + (long long)(b->tv_nsec - a->tv_nsec);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <host_IP> <puerto>\n", argv[0]);
        exit(1);
    }

    /* Los seis tamanios del enunciado: 10^1 .. 10^6 bytes.
       Este arreglo debe ser IDENTICO y en el mismo orden en el servidor. */
    const int tamanos[] = {10, 100, 1000, 10000, 100000, 1000000};
    const int num_tamanos = 6;

    int portno = atoi(argv[2]);
    
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

    /* Asincronismo: si el servidor todavia no hizo listen(), connect() falla
       con "Connection refused". En lugar de abortar, se reintenta una vez por
       segundo hasta MAX_INTENTOS. Despues de un connect() fallido el estado
       del socket queda indefinido, por eso en cada intento se cierra y se
       crea uno nuevo. El connect() sigue FUERA de todas las mediciones. */
    int sockfd;
    for (int intento = 1; ; intento++) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) error("Error abriendo socket");
        if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == 0)
            break;
        close(sockfd);
        if (intento == MAX_INTENTOS) error("Error conectando: el servidor no respondio");
        fprintf(stderr, "[CLIENTE] Servidor no disponible, reintento %d/%d\n",
                intento, MAX_INTENTOS);
        sleep(1);
    }

    /* Buffer reservado UNA sola vez, del tamanio mayor. Dentro de los bucles
       no se reserva memoria: eso contaminaria los tiempos. */
    int max_tam = tamanos[num_tamanos - 1];
    char *buffer = (char *) malloc(max_tam);
    if (buffer == NULL) error("Error al reservar memoria");
    memset(buffer, 'A', max_tam);

    /* Buffer de recepcion del eco, del tamanio mayor, reservado una sola vez */
    char *buffer_resp = (char *) malloc(max_tam);
    if (buffer_resp == NULL) error("Error al reservar memoria");

    printf("=== EJERCICIO 4 - TIEMPOS MEDIDOS EN EL CLIENTE (nanosegundos) ===\n");
    printf("Repeticiones contabilizadas por tamanio: %d\n", REPETICIONES);
    printf("Iteraciones de calentamiento descartadas: %d\n\n", CALENTAMIENTO);
    printf("%10s | %12s %12s %12s | %12s %12s %12s\n",
           "bytes", "write prom", "write min", "write max",
                    "read prom",  "read min",  "read max");
    printf("-----------+---------------------------------------"
           "+---------------------------------------\n");
    fflush(stdout);

    for (int i_tam = 0; i_tam < num_tamanos; i_tam++) {
        int tam_buffer = tamanos[i_tam];

        /* Acumuladores: se reinician al empezar cada tamanio */
        long long suma_w = 0, min_w = LLONG_MAX, max_w = 0;
        long long suma_r = 0, min_r = LLONG_MAX, max_r = 0;

        /* Las primeras CALENTAMIENTO vueltas se ejecutan igual que las demas
           pero no se registran. Sirven para que la ventana de congestion de
           TCP salga de slow-start, para que las paginas del buffer ya esten
           mapeadas y para que el codigo y los datos esten en cache. Sin esto
           la primera medicion de cada tamanio es un valor atipico que infla
           el promedio y se lleva el maximo. */
        for (int iter = 0; iter < CALENTAMIENTO + REPETICIONES; iter++) {
            struct timespec t0_write, t1_write;
            struct timespec t0_read,  t1_read;

            clock_gettime(CLOCK_MONOTONIC, &t0_write);
            int bytes_enviados = write(sockfd, buffer, tam_buffer);
            clock_gettime(CLOCK_MONOTONIC, &t1_write);

            if (bytes_enviados < 0) { error("Error en write");
            } else if (bytes_enviados != tam_buffer) {
                fprintf(stderr, "write parcial: se enviaron %d de %d bytes. "
                        "Experimento invalidado.\n", bytes_enviados, tam_buffer);
                exit(1);}

            /* ---------- lectura del eco: tam_buffer bytes ----------
               Se cronometra cada read() y luego se suma el tiempo, para asegurarnos de recibir toda la info. */
            int bytes_leidos = 0;
            long long ns_read_total = 0;
            while (bytes_leidos < tam_buffer) {
                clock_gettime(CLOCK_MONOTONIC, &t0_read);
                int n = read(sockfd, buffer_resp + bytes_leidos,
                             tam_buffer - bytes_leidos);
                clock_gettime(CLOCK_MONOTONIC, &t1_read);
                if (n <= 0) error("Error leyendo el eco del servidor");
                bytes_leidos += n;
                ns_read_total += ns_entre(&t0_read, &t1_read);
            }

            if (iter < CALENTAMIENTO) continue;   /* vuelta de calentamiento */

            long long ns_write = ns_entre(&t0_write, &t1_write);
            long long ns_read  = ns_read_total;

            suma_w += ns_write;
            if (ns_write < min_w) min_w = ns_write;
            if (ns_write > max_w) max_w = ns_write;

            suma_r += ns_read;
            if (ns_read < min_r) min_r = ns_read;
            if (ns_read > max_r) max_r = ns_read;
        }

        printf("%10d | %12lld %12lld %12lld | %12lld %12lld %12lld\n",
               tam_buffer,
               suma_w / REPETICIONES, min_w, max_w,
               suma_r / REPETICIONES, min_r, max_r);
        fflush(stdout);
    }

    free(buffer);
    free(buffer_resp);
    close(sockfd);
    return 0;
}
