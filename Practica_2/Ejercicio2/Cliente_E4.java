import java.io.OutputStream;
import java.net.Socket;
import java.util.Arrays;

public class Cliente_E3 {
    public static void main(String[] args) {
        if (args.length < 2) {
            System.err.println("Uso: java ClienteE3 <host> <puerto>");
            System.exit(1);
        }

        String host = args[0];
        int port = Integer.parseInt(args[1]);

        try (Socket socket = new Socket(host, port)) {
            OutputStream out = socket.getOutputStream();

            // Ráfagas de prueba: 10^1 a 10^6 bytes
            int[] tamanios = {10, 100, 1000, 10000, 100000, 1000000};

            for (int i = 0; i < tamanios.length; i++) {
                int tamano = tamanios[i];

                // Creación e inicialización del arreglo con la letra 'A'
                byte[] buffer = new byte[tamano];
                Arrays.fill(buffer, (byte) 'A');

                //CALCULO DE TIEMPO
                long t0 = System.nanoTime();
                // ESCRITURA EN EL STREAM
                out.write(buffer, 0, tamano);
                out.flush(); // Fuerza el envío inmediato de los bytes acumulados en el buffer de red
                //CALCULO RESULTADO DE TIEMPO
                long t1 = System.nanoTime();
                double tiempoWriteMs = (t1 - t0) / 1e6; // en milisegundos

                System.out.printf("[CLIENTE JAVA] Enviados exitosamente %d bytes (10^%d).\n", tamano, (i + 1), "TOTAL DE TIEMPO DE ENVIO(write) EN Milisegundos:", tiempoWriteMs);
                
                Thread.sleep(100); // Pausa de 100ms entre envíos
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}