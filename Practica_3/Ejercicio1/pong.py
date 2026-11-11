#!/usr/bin/env python3 
import sys 
import paho.mqtt.client as mqtt

BROKER = "192.168.56.10" if len(sys.argv) < 2 else sys.argv[1] 
PORT = 1883 
TOPIC_SUB = "test/ping" 
TOPIC_PUB = "test/pong" 
QOS = 0 # Se puede parametrizar a 0, 1 o 2

def on_connect(client, userdata, flags, rc): 
    if rc == 0: 
        print(f"[PONG] Conectado al broker. Suscribiendo a '{TOPIC_SUB}'...") 
        client.subscribe(TOPIC_SUB, qos=QOS) 
    else: 
        print(f"[PONG] Error al conectar, código: {rc}")

def on_message(client, userdata, msg):
    # Reenvía el mismo payload recibido hacia el tópico pong 
    client.publish(TOPIC_PUB, payload=msg.payload, qos=QOS)

def main(): 
    client = mqtt.Client(client_id="proceso_pong") 
    client.on_connect = on_connect 
    client.on_message = on_message 
    client.connect(BROKER, PORT, 60) 
    print("[PONG] Esperando mensajes ping...") 
    client.loop_forever()

if __name__ == "__main__": 
    main()