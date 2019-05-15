/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */
#ifdef __cplusplus
extern "C" {
#endif

#include "utils/base/time.h"
#include <assert.h>
#include <sys/timeb.h>

int64_t time_now_utc()
{
    return time(NULL);
}

int64_t time_now_local()
{
    // Логика похожа на NIX-ову версию. Но...В винде сдвиг часового пояса считается
    // в минутах и между UTC и лок. временем
    time_t utc = time(NULL);
    struct _timeb tb;

    _ftime( &tb );
    return utc - tb.timezone * 60;
}

int32_t time_tzoffset()
{
    struct _timeb tb;
    _ftime( &tb );
    return -tb.timezone * 60;
}

int time_local_from_utc(int64_t utc, struct tm * local)
{
    assert(local);
    const time_t tutc = utc; //для корректной работы в 32-х битных системах
    struct tm * ptm = localtime(&tutc);
    if(ptm)
        *local = *ptm;
    return ptm != NULL;
}

int64_t time_utc_from_local(const struct tm *local)
{
    assert(local);
    struct tm tmp = *local;
    return mktime(&tmp);
}

/*int time_local_dt(int64_t loctime, struct tm * result)
{
    time_t utc = loctime - time_tzoffset();
    struct tm * ptm = localtime(&utc);
    if(ptm)
        *result = *ptm;
    return ptm != NULL;
}*/

/*
Для проверки пересчета UTC -> Localtime я использовал эту программу. Она собирается
в gcc и mingw.
Вывод был таким:
./win.exe && ./lin
UTC: 1556020792
Offset: -300
Result:1556038792
UTC: 1556020792
Offset: 18000
Result: 1556038792


#include <time.h>
#include <stdio.h>

int main()
{
  time_t utc = time(0);
  struct tm ldt;

  printf("UTC: %d\n", utc);

#if defined(__linux__)
  localtime_r(&utc, &ldt);
  printf("Offset: %d\n", ldt.tm_gmtoff);
  printf("Result: %d\n", utc + ldt.tm_gmtoff);
#else
  struct _timeb tb;
  _ftime( &tb );
  printf("Offset: %d\n", tb.timezone);
  printf("Result:%d\n", utc - tb.timezone * 60);
#endif

  return 0;
}
*/

#ifdef __cplusplus
}
#endif
