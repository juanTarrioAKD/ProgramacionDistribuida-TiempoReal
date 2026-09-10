import java.io.InputStream;
import java.net.ServerSocket;
import java.net.Socket;

public class Servidor_E3 {
    public static void main(String[] args) {
        if (args.length < 1) {
            System.err.println("Uso: java ServidorE3 <puerto>");
            System.exit(1);
        }

        int port = Integer.parseInt(args[0]);

        // Instanciación del ServerSocket (ejecuta socket + bind + listen en bajo nivel)
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("[SERVIDOR JAVA] Escuchando en el puerto " + port + "...");

            // Bloqueo a la espera del cliente
            Socket clientSocket = serverSocket.accept();
            System.out.println("[SERVIDOR JAVA] Cliente conectado desde: " + clientSocket.getInetAddress());

            InputStream in = clientSocket.getInputStream();

            // Ráfagas de prueba: 10^1 a 10^6 bytes
            int[] tamanios = {10, 100, 1000, 10000, 100000, 1000000};

            for (int i = 0; i < tamanios.length; i++) {
                int tamanoEsperado = tamanios[i];
                byte[] buffer = new byte[tamanoEsperado]; // Reserva dinámica de arreglo de bytes

                int totalRecibido = 0;

                // BUCLE DE LECTURA PARCIAL (Equivalente al read() en C)
                while (totalRecibido < tamanoEsperado) {
                    // read(byte[] b, int off, int len) lee hasta len bytes y los guarda desde la posición off
                    int bytesLeidos = in.read(buffer, totalRecibido, tamanoEsperado - totalRecibido);
                    if (bytesLeidos == -1) break; // Fin de flujo (EOF)
                    totalRecibido += bytesLeidos;
                }

                // VERIFICACIÓN INTEGRIDAD DE DATOS (Sin imprimir en pantalla)
                boolean datosCorrectos = true;
                for (int j = 0; j < totalRecibido; j++) {
                    if (buffer[j] != (byte) 'A') {
                        datosCorrectos = false;
                        break;
                    }
                }

                if (totalRecibido == tamanoEsperado && datosCorrectos) {
                    System.out.printf("[SERVIDOR JAVA] [OK] Recibidos correctamente %d bytes (10^%d).\n", totalRecibido, (i + 1));
                } else {
                    System.out.printf("[SERVIDOR JAVA] [ERROR] Falla en prueba de %d bytes. Recibidos: %d\n", tamanoEsperado, totalRecibido);
                }
            }

            clientSocket.close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}