#!/bin/bash
# Простой тест для проверки вывода при отсутсвующей связи
OUT=/tmp/out
BINDIR="$1"

echo "<-- SYNC TEST SUITE -->"
echo "Binary dir: ${BINDIR}"

echo "Test #1"
${BINDIR}/utils/sync/tekon_sync -a udp:127.0.0.1:51960@9 -d 3:0xF017:0xF018 -t100 -p00000001 -u 0 &> ${OUT}
grep 'receiving error' /tmp/out || exit 1
grep 'date/time reading failed' /tmp/out || exit 1
rm ${OUT}



