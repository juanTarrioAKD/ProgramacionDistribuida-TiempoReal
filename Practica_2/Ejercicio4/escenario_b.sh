#!/bin/bash
# Escenario b: servidor en la VM vma (192.168.56.10), cliente en el host.
# Requiere gcc en el host.
# Uso (desde Ejercicio4):  ./escenario_b.sh
set -e
trap 'echo "[FALLO] linea $LINENO, codigo $?"' ERR
cd "$(dirname "$0")"

PUERTO=5001
IP_SERVIDOR=192.168.56.10          # IP host-only de vma
FECHA=$(date +%Y%m%d_%H%M%S)
mkdir -p resultados build

# "< /dev/null" en cada "vagrant ssh": evita que ssh consuma el resto del script

# 1) Levantar unicamente la VM del servidor
vagrant up vma

# 2) Compilar: servidor en la VM, cliente en el host
vagrant ssh vma -c "gcc -O2 -Wall -o servidor_e4 /vagrant/Ejercicio4/src/servidor_e4.c" < /dev/null
gcc -O2 -Wall -o build/cliente_e4 src/cliente_e4.c

# 3) Servidor en segundo plano (pkill libera el puerto, setsid lo desprende de ssh,
#    pgrep verifica que haya arrancado)
vagrant ssh vma -c "pkill -x servidor_e4 || true" < /dev/null
vagrant ssh vma -c "setsid nohup ./servidor_e4 $PUERTO > servidor.log 2>&1 < /dev/null & sleep 2; pgrep -x servidor_e4 > /dev/null || { echo '[ERROR] El servidor no arranco. Contenido del log:'; cat servidor.log; exit 1; }" < /dev/null
echo "[OK] Servidor activo en $IP_SERVIDOR:$PUERTO"

# 4) Cliente en el host, en primer plano
./build/cliente_e4 "$IP_SERVIDOR" "$PUERTO" > "resultados/b_cliente_$FECHA.txt"

# 5) Traer el log del servidor al repositorio
vagrant ssh vma -c "cat servidor.log" < /dev/null > "resultados/b_servidor_$FECHA.log"

echo
cat "resultados/b_cliente_$FECHA.txt"
echo
echo "Resultados guardados en:"
echo "  resultados/b_cliente_$FECHA.txt"
echo "  resultados/b_servidor_$FECHA.log"
