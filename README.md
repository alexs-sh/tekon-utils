[![Total alerts](https://img.shields.io/lgtm/alerts/b/alexsteam4000/tekon-utils-pub.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/b/alexsteam4000/tekon-utils-pub/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/b/alexsteam4000/tekon-utils-pub.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/b/alexsteam4000/tekon-utils-pub/context:cpp)
[![codecov](https://codecov.io/bb/alexsteam4000/tekon-utils-pub/branch/master/graph/badge.svg)](https://codecov.io/bb/alexsteam4000/tekon-utils-pub)

# TEKON-UTILS

Набор утилит для работы со счетчиками [Тэкон](https://kreit.ru/) через Ethernet контроллер [К-104](https://kreit.ru/products/communication/k-104.html). 
Позволяют выполнять чтение значений, чтение архивов, синхронизацию времени.
Для работы используется протокол описанный в [Т10.06.59РД](https://kreit.ru/files/prot17_6.pdf) и [Т10.06.59РД-Д1](https://kreit.ru/files/prot_d1.pdf).

## Быстрый старт

### Чтение параметров
```console
tekon_msr -a udp:10.0.0.3:51960@9 -p'3:0xF001:0:H 3:0xF017:0:D 3:0xF018:0:T 3:0x801c:2:f'
9:3:0xf001:0 H 0x3840 OK 1558082153 18000
9:3:0xf017:0 D 2019-05-17 OK 1558082153 18000
9:3:0xf018:0 T 13:35:49 OK 1558082153 18000
9:3:0x801c:2 F 38.151833 OK 1558082153 18000
```

### Чтение архива
```console
tekon_arch -a udp:10.0.0.3:51960@9 -p 3:0x801C:0:12:F  -i m:12   -d 3:0xF017:0xF018 
9:3:0x801c:0 F -nan OK 1546282800 18000
9:3:0x801c:1 F -nan OK 1548961200 18000
9:3:0x801c:2 F 38.151833 OK 1551380400 18000
...
```

### Синхронизация времени
```console
tekon_sync -a udp:10.0.0.3:51960@9 -d 3:0xF017:0xF018 -p 00000001
```

## Сборка

Для сборки потребуются:

* gcc
* cmake 
* make

### Debian x86_64
```console
apt install git cmake make gcc
git clone  <repo>
cd tekon-utils/
mkdir build
cd build
cmake ..
make 
make test
```

### Debian ARM
```console
apt install git cmake make gcc-arm-linux-gnueabihf
git clone  <repo>
cd tekon-utils/
mkdir build-armhf
cd build-armhf
cmake -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc -DCMAKE_FIND_ROOT_PATH=/usr/arm-linux-gnueabihf ..
make 
```

### Windows XP x86
```console
apt install git cmake make gcc-mingw-w64-i686
git clone  <repo>
cd tekon-utils/
mkdir build-winxp-x86
cd build-winxp-x86
cmake -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc -DCMAKE_LINKER=/usr/i686-w64-mingw32/bin/ld  -DCMAKE_C_STANDARD_LIBRARIES="-lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32 -lws2_32 -liphlpapi -lpsapi -luserenv" -DCMAKE_C_FLAGS_RELEASE="-DWINVER=0x0501" -DCMAKE_EXE_LINKER_FLAGS="-Wl,-Bstatic -lwinpthread" ..
make 
```

Все примеры выполнены в ОС Debian 9 x86_64.

git clone  <repo> - вместо параметра <repo> следует подставить адрес репозитория, 
например, https://github.com/alexs-sh/tekon-utils.git

## Применение

Утилиты для чтения получают данные с устройства и выводят их в stdout. Чтение
может выполняться по tcp или udp. К значениям добавляется информация о том, 
откуда они были получены, качество и метки времени.

Качество имеет сл. значения:
  
* OK - данные получены и имеют достоверное значение
* INV - данные получены, но имеют недостоверное значение 
* COM - обрыв связи
* UNK - неопределенное значение

Метки времени отображаются в UTC. Дополнительно с ними хранится информация о
часовом поясе (TZ). TZ отображается в виде сдвига локального времени от UTC в
секундах.

### Чтение значений

Формат вывода данных:

шлюз:адрес:параметр:индекс тип значение качество UTC TZ
```console
9:3:0xf001:0 H 0x3840 OK 1557900053 18000
```

### Чтение архива

Формат вывода данных:

шлюз:адрес:параметр:индекс тип значение качество UTC TZ
```console
9:3:0x801c:0 F -nan OK 1546282800 18000
```

Чтение архива может выполнятся в 2-х вариантах: 

* без метки времени
* с меткой времени


Чтение без меток времени. Все значения UTC равны -1.
```console
tekon_arch -a udp:10.0.0.3:51960@9 -p 3:0x801C:0:12:F 
9:3:0x801c:0 F -nan OK -1 18000
9:3:0x801c:1 F -nan OK -1 18000
9:3:0x801c:2 F 38.151833 OK -1 18000
...
```

Чтение с метками времени. Требует дополнительных параметров с описанием
интервала и адресов даты / времени. Переводит индексы Тэкон в UTC время.
```console
tekon_arch -a udp:10.0.0.3:51960@9 -p 3:0x801C:0:12:F -i m:12 -d 3:0xF017:0xF018
9:3:0x801c:0 F -nan OK 1546282800 18000
9:3:0x801c:1 F -nan OK 1548961200 18000
9:3:0x801c:2 F 38.151833 OK 1551380400 18000
...
```

Для месячных, суточных, часовых архивов метка времени относится к началу архива.

Для интервальных архивов метка времени относится к концу архива.

Более подробно см. [Т10.06.59РД-Д1](https://kreit.ru/files/prot_d1.pdf) стр.
31-33

### Синхронизация времени

Синхронизация времени имеет несколько подводных камней:

* в документации нет описания алгоритма синхронизации
* разработчики не предоставляют какой-либо информации на форумах
* перевод времени может приводить к изменения индексов архивов, что в свою
    очередь приведет к повреждению архивов

Механизм синхронизации предусматривает ряд проверок, позволяющих снизить
вероятность повреждения архивов. Каждая проверка может быть включена или
отключена.

* разность времени. Если время в устройстве отличается от нового больше, чем на
    заданное значение, то синхронизация не будет выполнена
* индексы. Если индексы (месячный (12), суточный, часовой (1536),
    интервальный (5 мин) ) для времени устройства не совпадают с индексами для
    нового времени, то синхронизация не будет выполнена
* переход через минуты. Если время устройства содержит не такое количество минутных
    интервалов, как новое время, то синхронизация не будет выполнена

Разность времени устройства и нового времени больше 100 сек.
```console
tekon_sync -a udp:10.0.0.3:51960@9 -d 3:0xF017:0xF018 -p 00000001 -u 1557897094 -c 'difference:100'
tekon_sync : ERR : Difference check failed
```

Индексы для времени устройства и нового времени не равны.
```console
tekon_sync -a udp:10.0.0.3:51960@9 -d 3:0xF017:0xF018 -p 00000001 -u 1557897094 -c 'indexes'   
tekon_sync : ERR : Indexes check failed
```

Кол-во 10-ти минутных интервалов во времени устройства и новом времени не равно.
```console
tekon_sync -a udp:10.0.0.3:51960@9 -d 3:0xF017:0xF018 -p 00000001 -u 1557897094 -c 'minutes:10'
tekon_sync : ERR : Minutes check failed
```

Объединение проверок.
```console
tekon_sync -a udp:10.0.0.3:51960@9 -d 3:0xF017:0xF018 -p 00000001 -u 1557897094 -c 'difference:100 minutes:10 indexes'
tekon_sync : ERR : Difference check failed
```

Отключение проверок. 
```console
tekon_sync -a udp:10.0.0.3:51960@9 -d 3:0xF017:0xF018 -p 00000001 -u 1557897094 -c 'none'
```
