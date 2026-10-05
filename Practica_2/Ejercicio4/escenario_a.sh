#!/bin/bash
# Escenario a: dos maquinas virtuales.
# Uso (desde Ejercicio4):  ./escenario_a.sh
set -e
cd "$(dirname "$0")"

PUERTO=5000
IP_SERVIDOR=192.168.56.10
FECHA=$(date +%Y%m%d_%H%M%S)
mkdir -p resultados

# 1) Levantar las VMs (la primera vez las crea y las aprovisiona)
vagrant up vma vmb

# 2) Compilar dentro de cada VM, desde el codigo compartido en /vagrant
vagrant ssh vma -c "gcc -O2 -Wall -o servidor_e4 /vagrant/Ejercicio4/src/servidor_e4.c"
vagrant ssh vmb -c "gcc -O2 -Wall -o cliente_e4 /vagrant/Ejercicio4/src/cliente_e4.c"

# 3) Servidor en segundo plano. Primero se termina cualquier servidor de una
#    corrida anterior que haya quedado vivo (tendria el puerto ocupado).
#    nohup y las redirecciones desacoplan el proceso de la sesion ssh: sin eso
#    muere al cerrarse la sesion, o vagrant ssh queda esperando.
vagrant ssh vma -c "pkill -x servidor_e4 || true"
vagrant ssh vma -c "nohup ./servidor_e4 $PUERTO > /vagrant/Ejercicio4/resultados/a_servidor_$FECHA.log 2>&1 < /dev/null &"

# 4) Cliente en primer plano. No hace falta esperar al servidor: si el cliente
#    llega antes que el listen(), reintenta el connect() (asincronismo).
vagrant ssh vmb -c "./cliente_e4 $IP_SERVIDOR $PUERTO > /vagrant/Ejercicio4/resultados/a_cliente_$FECHA.txt"

cat "resultados/a_cliente_$FECHA.txt"
echo "Resultados: resultados/a_cliente_$FECHA.txt y resultados/a_servidor_$FECHA.log"
