#!/usr/bin/env python3
import time
import paho.mqtt.client as mqtt

# Configuración del Broker Público HiveMQ
BROKER = "broker.hivemq.com"
PORT = 1883

# Mismo tópico acordado con el suscriptor
TOPIC = "pdytr/2026/ejercicio3/datos"
CLIENT_ID = "pdytr_pub_alumno"


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"[PUB] Conectado exitosamente a {BROKER}:{PORT}")
    else:
        print(f"[PUB] Fallo de conexion. Codigo: {rc}")


def main():
    print(f"[PUB] Iniciando publicador hacia {BROKER}...")
    client = mqtt.Client(client_id=CLIENT_ID)
    client.on_connect = on_connect
    client.connect(BROKER, PORT, keepalive=60)

    # Iniciamos el bucle en segundo plano para procesar la conexion TCP/MQTT
    client.loop_start()

    # Pausa breve para asegurar que el handshake con el broker en Internet finalice
    time.sleep(2)

    mensajes = [
        "Primer mensaje de prueba con HiveMQ",
        "Probando comunicacion a traves de Internet",
        "Demostrando el desacoplamiento espacial con MQTT",
    ]

    for i, texto in enumerate(mensajes, start=1):
        print(f"[PUB] Enviando ({i}/{len(mensajes)}): '{texto}'")
        client.publish(TOPIC, payload=texto, qos=0)
        time.sleep(1)

    print("[PUB] Finalizados los envios. Esperando vaciado de colas...")
    time.sleep(2)  # Pausa fundamental para que la red remota termine de transmitir

    client.loop_stop()
    client.disconnect()
    print("[PUB] Desconectado con exito.")


if __name__ == "__main__":
    main()