#!/bin/bash
# ===========================================================================
# Escenario b: una maquina virtual y el host.
#   - servidor: dentro de la VM vma (192.168.56.10)
#   - cliente : en el host, que alcanza a la VM por la red host-only
#               (el host es 192.168.56.1 en esa red)
#
# Requiere gcc en el host, asi que debe ejecutarse desde una computadora con
# Linux nativo que tenga instalados VirtualBox y Vagrant.
#
# Uso (desde Ejercicio4):  ./escenario_b.sh
# ===========================================================================
set -e
trap 'echo "[FALLO] linea $LINENO, codigo $?"' ERR
cd "$(dirname "$0")"

PUERTO=5001
IP_SERVIDOR=192.168.56.10          # IP host-only de vma
FECHA=$(date +%Y%m%d_%H%M%S)
mkdir -p resultados build

# ---------------------------------------------------------------------------
# NOTA sobre "< /dev/null":
# ssh lee stdin apenas arranca. Dentro de un script eso hace que se trague el
# resto del archivo: bash se queda sin lineas que ejecutar y el script termina
# en silencio, sin error y con codigo 0. Redirigir la entrada desde /dev/null
# en CADA llamada a "vagrant ssh" lo evita.
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# 1) Levantar unicamente la VM del servidor
# ---------------------------------------------------------------------------
vagrant up vma

# ---------------------------------------------------------------------------
# 2) Compilar: el servidor dentro de la VM, el cliente en el host
# ---------------------------------------------------------------------------
vagrant ssh vma -c "gcc -O2 -Wall -o servidor_e4 /vagrant/Ejercicio4/src/servidor_e4.c" < /dev/null
gcc -O2 -Wall -o build/cliente_e4 src/cliente_e4.c

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
# 4) Cliente en el host, en primer plano. El resultado se escribe directamente
#    en el repositorio, sin pasar por la carpeta compartida.
#    El cliente conserva su bucle de reintentos de connect(): es la solucion al
#    problema de asincronismo que pide la consigna, y cubre el caso de que el
#    proceso haya arrancado pero todavia no haya llegado al listen().
# ---------------------------------------------------------------------------
./build/cliente_e4 "$IP_SERVIDOR" "$PUERTO" > "resultados/b_cliente_$FECHA.txt"

# ---------------------------------------------------------------------------
# 5) Traer el log del servidor al repositorio
# ---------------------------------------------------------------------------
vagrant ssh vma -c "cat servidor.log" < /dev/null > "resultados/b_servidor_$FECHA.log"

echo
cat "resultados/b_cliente_$FECHA.txt"
echo
echo "Resultados guardados en:"
echo "  resultados/b_cliente_$FECHA.txt"
echo "  resultados/b_servidor_$FECHA.log"
