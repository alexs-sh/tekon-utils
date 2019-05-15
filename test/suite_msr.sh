#!/bin/bash
# Простой тест для проверки вывода при отсутсвующей связи
OUT=/tmp/out
BINDIR="$1"
TEST_CNT=1

fail()
{
  echo $1
  exit 1
}

test_output()
{
  echo -n "TEST #${TEST_CNT} : $1..."
  TEST_CNT=$((TEST_CNT + 1))
  # Проверка кол-ва строк
  LN=$(wc -l ${OUT} | cut -f 1 -d' ')
  EXPECT=6
  if [ "$LN" -ne "$EXPECT" ]; then
    fail "$1 Invalid output. Got ${LN} lines insted of ${EXPECT}"
  fi

  # Проверка кол-ва слов
  WD=$(wc -w ${OUT} | cut -f 1 -d' ')
  EXPECT=36
  if [ "$WD" -ne "$EXPECT" ]; then
    fail "$1 Invalid output. Got ${WD} words insted of ${EXPECT}"
  fi
  
  # Проверка по шаблону строки
  PAT='2:3:0x8001 F 0.000000 COM'
  grep "${PAT}" /tmp/out > /dev/null || fail "Invalid output. Can't find ${PAT}"

  PAT='2:3:0x8001 R 0x00000000 COM'
  grep "${PAT}" /tmp/out > /dev/null || fail "Invalid output. Can't find ${PAT}"

  PAT='2:3:0x8001 U 0 COM'
  grep "${PAT}" /tmp/out > /dev/null || fail "Invalid output. Can't find ${PAT}"

  PAT='2:3:0x8001 D 2000-00-00 COM'
  grep "${PAT}" /tmp/out > /dev/null || fail "Invalid output. Can't find ${PAT}"

  PAT='2:3:0x8001 T 00:00:00 COM'
  grep "${PAT}" /tmp/out > /dev/null || fail "Invalid output. Can't find ${PAT}"

  PAT='2:3:0x8001 H 0x0 COM'
  grep "${PAT}" /tmp/out > /dev/null || fail "Invalid output. Can't find ${PAT}"

  echo "Done"
}

echo "<-- MEASURMENT TEST SUITE -->"
echo "Binary dir: ${BINDIR}"

${BINDIR}/utils/msr/tekon_msr -a udp:127.0.0.1:59160@2 -p'3:0x8001:0:F 3:0x8001:0:R 3:0x8001:0:U 3:0x8001:0:H 3:0x8001:0:D 3:0x8001:0:T' -t 100 > ${OUT} 2>/dev/null
test_output "UDP"
rm ${OUT}

${BINDIR}/utils/msr/tekon_msr -a tcp:127.0.0.1:59160@2 -p'3:0x8001:0:F 3:0x8001:0:R 3:0x8001:0:U 3:0x8001:0:H 3:0x8001:0:D 3:0x8001:0:T' -t 100 > ${OUT} 2>/dev/null
test_output "TCP"
rm ${OUT}

