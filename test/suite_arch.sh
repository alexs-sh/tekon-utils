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
  EXPECT=12
  if [ "$LN" -ne "$EXPECT" ]; then
    fail "Invalid output. Got ${LN} lines insted of ${EXPECT}"
  fi
  
  # Проверка кол-ва слов
  WD=$(wc -w ${OUT} | cut -f 1 -d' ')
  EXPECT=72
  if [ "$WD" -ne "$EXPECT" ]; then
    fail "Invalid output. Got ${WD} words insted of ${EXPECT}"
  fi
  # Проверка по шаблону строки
  PAT='2:3:0x801c:([0-9])+ F 0.000000 COM'
  grep -E "${PAT}" /tmp/out > /dev/null && echo "Done" || fail "Invalid output. Can't find ${PAT}"
}

echo "<-- ARCHIVE TEST SUITE -->"
echo "Binary dir: ${BINDIR}"

./utils/arch/tekon_arch ./utils/arch/tekon_arch -a udp:127.0.0.1:51960@2 -p 3:0x801C:0:12:F -t 100 > ${OUT} 2>/dev/null
test_output "UDP"
rm ${OUT}

./utils/arch/tekon_arch ./utils/arch/tekon_arch -a tcp:127.0.0.1:51960@2 -p 3:0x801C:0:12:F -t 100 > ${OUT} 2>/dev/null
test_output "TCP"
rm ${OUT}




