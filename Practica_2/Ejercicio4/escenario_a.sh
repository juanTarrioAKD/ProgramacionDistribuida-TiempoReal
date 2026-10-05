#!/bin/bash
# ===========================================================================
# Escenario a: dos maquinas virtuales, un proceso de comunicaciones en cada una.
# Uso (desde Ejercicio4, en Git Bash):  ./escenario_a.sh
# ===========================================================================
set -e
trap 'echo "[FALLO] linea $LINENO, codigo $?"' ERR
cd "$(dirname "$0")"

PUERTO=5000
IP_SERVIDOR=192.168.56.10          # IP host-only de vma
FECHA=$(date +%Y%m%d_%H%M%S)
mkdir -p resultados

# ---------------------------------------------------------------------------
# NOTA sobre "< /dev/null":
# ssh lee stdin apenas arranca. Dentro de un script eso hace que se trague el
# resto del archivo: bash se queda sin lineas que ejecutar y el script termina
# en silencio, sin error y con codigo 0. Redirigir la entrada desde /dev/null
# en CADA llamada a "vagrant ssh" lo evita.
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# 1) Levantar las VMs (la primera vez las crea y las aprovisiona)
# ---------------------------------------------------------------------------
vagrant up vma vmb

# ---------------------------------------------------------------------------
# 2) Compilar dentro de cada VM, desde el codigo compartido en /vagrant
# ---------------------------------------------------------------------------
vagrant ssh vma -c "gcc -O2 -Wall -o servidor_e4 /vagrant/Ejercicio4/src/servidor_e4.c" < /dev/null
vagrant ssh vmb -c "gcc -O2 -Wall -o cliente_e4  /vagrant/Ejercicio4/src/cliente_e4.c"  < /dev/null

# ---------------------------------------------------------------------------
# 3) Servidor en segundo plano dentro de vma.
#
#    - pkill  : termina cualquier servidor de una corrida anterior que haya
#               quedado vivo y tendria el puerto ocupado.
#    - setsid : desprende el proceso del grupo de la sesion ssh. Sin esto el
#               proceso muere cuando "vagrant ssh" cierra la conexion.
#    - El log va al home de la VM, no a la carpeta compartida: un proceso
#      desprendido escribiendo sobre vboxsf es una complicacion innecesaria.
#      Se copia al repositorio en el paso 5.
#    - sleep + pgrep: dan tiempo a que el proceso se desprenda y VERIFICAN que
#      haya arrancado. Si fallo, el script aborta aca mismo mostrando el log,
#      en lugar de dejar al cliente reintentando contra la nada.
# ---------------------------------------------------------------------------
vagrant ssh vma -c "pkill -x servidor_e4 || true" < /dev/null
vagrant ssh vma -c "setsid nohup ./servidor_e4 $PUERTO > servidor.log 2>&1 < /dev/null & sleep 2; pgrep -x servidor_e4 > /dev/null || { echo '[ERROR] El servidor no arranco. Contenido del log:'; cat servidor.log; exit 1; }" < /dev/null
echo "[OK] Servidor activo en $IP_SERVIDOR:$PUERTO"

# ---------------------------------------------------------------------------
# 4) Cliente en primer plano dentro de vmb.
#    Aunque el servidor ya quedo verificado, el cliente conserva su bucle de
#    reintentos de connect(): es la solucion al problema de asincronismo que
#    pide la consigna, y cubre el caso de que el proceso haya arrancado pero
#    todavia no haya llegado al listen().
# ---------------------------------------------------------------------------
vagrant ssh vmb -c "./cliente_e4 $IP_SERVIDOR $PUERTO > /vagrant/Ejercicio4/resultados/a_cliente_$FECHA.txt" < /dev/null

# ---------------------------------------------------------------------------
# 5) Traer el log del servidor al repositorio
# ---------------------------------------------------------------------------
vagrant ssh vma -c "cat servidor.log" < /dev/null > "resultados/a_servidor_$FECHA.log"

echo
cat "resultados/a_cliente_$FECHA.txt"
echo
echo "Resultados guardados en:"
echo "  resultados/a_cliente_$FECHA.txt"
echo "  resultados/a_servidor_$FECHA.log"
