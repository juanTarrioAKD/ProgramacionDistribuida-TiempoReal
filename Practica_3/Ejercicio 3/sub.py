#!/usr/bin/env python3
import time
import paho.mqtt.client as mqtt

# Configuración del Broker Público HiveMQ
BROKER = "broker.hivemq.com"
PORT = 1883

# Usamos un prefijo único para no colisionar con otros usuarios en el broker público
TOPIC = "pdytr/2026/ejercicio3/datos"
CLIENT_ID = "pdytr_sub_alumno"


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"[SUB] Conectado exitosamente a {BROKER}:{PORT}")
        print(f"[SUB] Suscribiendo al topico: {TOPIC}")
        client.subscribe(TOPIC, qos=0)
    else:
        print(f"[SUB] Fallo al conectar. Codigo de error: {rc}")


def on_message(client, userdata, msg):
    contenido = msg.payload.decode("utf-8", errors="replace")
    print(f"[SUB] Mensaje recibido en [{msg.topic}]: {contenido}")


def main():
    print(f"[SUB] Iniciando cliente suscriptor contra {BROKER}...")
    client = mqtt.Client(client_id=CLIENT_ID)
    client.on_connect = on_connect
    client.on_message = on_message

    try:
        client.connect(BROKER, PORT, keepalive=60)
        # loop_forever mantiene el proceso escuchando eventos de red
        client.loop_forever()
    except KeyboardInterrupt:
        print("\n[SUB] Deteniendo cliente...")
    finally:
        client.disconnect()
        print("[SUB] Desconectado.")


if __name__ == "__main__":
    main()