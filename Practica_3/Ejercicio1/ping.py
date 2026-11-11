#!/usr/bin/env python3
import sys
import time
import threading
import paho.mqtt.client as mqtt

BROKER = "192.168.56.10" if len(sys.argv) < 2 else sys.argv[1]
PORT = 1883
TOPIC_PUB = "test/ping"
TOPIC_SUB = "test/pong"
QOS = 0  # Probar con QoS 0, 1 o 2

# Bandera y timestamp para sincronización entre el callback y el hilo principal
mensaje_recibido = threading.Event()
t1 = 0.0


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        client.subscribe(TOPIC_SUB, qos=QOS)


def on_message(client, userdata, msg):
    global t1
    # Se toma el tiempo inmediatamente tras la recepción en espacio de usuario
    t1 = time.monotonic()
    mensaje_recibido.set()


def main():
    client = mqtt.Client(client_id="proceso_ping")
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(BROKER, PORT, 60)

    # Iniciamos el bucle de red de paho en segundo plano
    client.loop_start()

    # Damos una breve pausa para asegurar que la suscripción esté lista en el broker
    time.sleep(1)

    tamanios = [2, 3]
    print(f"{'Tamaño (Bytes)':<18}{'RTT Total (s)':<18}{'One-Way (s)':<18}")
    print("-" * 54)

    for tam in tamanios:
        # Preparamos el payload con bytes patrón (ej: 'A')
        payload = b"A" * tam
        mensaje_recibido.clear()

        # Medición: t0 inmediatamente antes de publicar
        t0 = time.monotonic()
        client.publish(TOPIC_PUB, payload=payload, qos=QOS)

        # Esperar hasta que llegue la respuesta en on_message
        if mensaje_recibido.wait(timeout=10.0):
            rtt = t1 - t0
            one_way = rtt / 2.0
            print(f"{tam:<18}{rtt:<18.6f}{one_way:<18.6f}")
        else:
            print(f"{tam:<18}{'TIMEOUT':<18}{'TIMEOUT':<18}")

        time.sleep(0.2)  # Pausa de estabilización entre pruebas

    client.loop_stop()
    client.disconnect()


if __name__ == "__main__":
    main()