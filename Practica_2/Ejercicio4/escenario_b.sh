#!/bin/bash
# Escenario b: una maquina virtual y el host.
# Uso (desde Ejercicio4):  ./escenario_b.sh      Requiere gcc en el host.
set -e
cd "$(dirname "$0")"

PUERTO=5001
IP_SERVIDOR=192.168.56.10
FECHA=$(date +%Y%m%d_%H%M%S)
mkdir -p resultados build

# 1) Levantar solo la VM del servidor
vagrant up vma

# 2) Compilar: el servidor dentro de la VM, el cliente en el host
vagrant ssh vma -c "gcc -O2 -Wall -o servidor_e4 /vagrant/Ejercicio4/src/servidor_e4.c"
gcc -O2 -Wall -o build/cliente_e4 src/cliente_e4.c

# 3) Servidor en segundo plano en la VM (mismo criterio que el escenario a)
vagrant ssh vma -c "pkill -x servidor_e4 || true"
vagrant ssh vma -c "nohup ./servidor_e4 $PUERTO > /vagrant/Ejercicio4/resultados/b_servidor_$FECHA.log 2>&1 < /dev/null &"

# 4) Cliente en el host: el resultado se escribe directamente en el host
./build/cliente_e4 "$IP_SERVIDOR" "$PUERTO" > "resultados/b_cliente_$FECHA.txt"

cat "resultados/b_cliente_$FECHA.txt"
echo "Resultados: resultados/b_cliente_$FECHA.txt y resultados/b_servidor_$FECHA.log"
