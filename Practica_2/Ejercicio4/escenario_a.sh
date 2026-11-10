#!/bin/bash
# Escenario a: dos VMs, servidor en vma y cliente en vmb.
# Uso (desde Ejercicio4, en Git Bash):  ./escenario_a.sh
set -e
trap 'echo "[FALLO] linea $LINENO, codigo $?"' ERR
cd "$(dirname "$0")"

PUERTO=5000
IP_SERVIDOR=192.168.56.10          # IP host-only de vma
FECHA=$(date +%Y%m%d_%H%M%S)
mkdir -p resultados

# "< /dev/null" en cada "vagrant ssh": evita que ssh consuma el resto del script

# 1) Levantar las VMs
vagrant up vma vmb

# 2) Compilar dentro de cada VM
vagrant ssh vma -c "gcc -O2 -Wall -o servidor_e4 /vagrant/Ejercicio4/src/servidor_e4.c" < /dev/null
vagrant ssh vmb -c "gcc -O2 -Wall -o cliente_e4  /vagrant/Ejercicio4/src/cliente_e4.c"  < /dev/null

# 3) Servidor en segundo plano (pkill libera el puerto, setsid lo desprende de ssh,
#    pgrep verifica que haya arrancado)
vagrant ssh vma -c "pkill -x servidor_e4 || true" < /dev/null
vagrant ssh vma -c "setsid nohup ./servidor_e4 $PUERTO > servidor.log 2>&1 < /dev/null & sleep 2; pgrep -x servidor_e4 > /dev/null || { echo '[ERROR] El servidor no arranco. Contenido del log:'; cat servidor.log; exit 1; }" < /dev/null
echo "[OK] Servidor activo en $IP_SERVIDOR:$PUERTO"

# 4) Cliente en primer plano dentro de vmb
vagrant ssh vmb -c "./cliente_e4 $IP_SERVIDOR $PUERTO > /vagrant/Ejercicio4/resultados/a_cliente_$FECHA.txt" < /dev/null

# 5) Traer el log del servidor al repositorio
vagrant ssh vma -c "cat servidor.log" < /dev/null > "resultados/a_servidor_$FECHA.log"

echo
cat "resultados/a_cliente_$FECHA.txt"
echo
echo "Resultados guardados en:"
echo "  resultados/a_cliente_$FECHA.txt"
echo "  resultados/a_servidor_$FECHA.log"
