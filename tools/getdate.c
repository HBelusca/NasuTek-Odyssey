/*
 * PROJECT:     RosBE - ReactOS Build Environment for Windows
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        Tools/getdate.c
 * PURPOSE:     Returns System Date
 * COPYRIGHT:   Copyright 2007 Peter Ward <dralnix@gmail.com>
 *
 */


#include <windows.h>
#include <stdio.h>

int main(void)
{
    SYSTEMTIME UTCSystemTime;
    SYSTEMTIME LocalSystemTime;

    char CurrentDate[20];
    char CurrentTime[20];

    GetSystemTime(&UTCSystemTime);
    SystemTimeToTzSpecificLocalTime(NULL, &UTCSystemTime, &LocalSystemTime);

    GetDateFormat(LOCALE_USER_DEFAULT,
                  0,
                  &LocalSystemTime,
                  "yyMMdd",
                  CurrentDate,
                  20);
    GetTimeFormat(LOCALE_USER_DEFAULT,
                  0,
                  &LocalSystemTime,
                  "HHmm",
                  CurrentTime,
                  20);

    printf("%s-%s", CurrentDate, CurrentTime);

    return 0;
}
