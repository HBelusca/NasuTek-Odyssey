/*
 * Copyright (C) 2004 Eric Kohl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


typedef struct _TZ_INFO
{
  LONG Bias;
  LONG StandardBias;
  LONG DaylightBias;
  SYSTEMTIME StandardDate;
  SYSTEMTIME DaylightDate;
} TZ_INFO, *PTZ_INFO;

typedef struct _TIMEZONE_ENTRY
{
  struct _TIMEZONE_ENTRY *Prev;
  struct _TIMEZONE_ENTRY *Next;
  WCHAR Description[64];   /* 'Display' */
  WCHAR StandardName[32];  /* 'Std' */
  WCHAR DaylightName[32];  /* 'Dlt' */
  TZ_INFO TimezoneInfo;    /* 'TZI' */
  ULONG Index;
} TIMEZONE_ENTRY, *PTIMEZONE_ENTRY;

typedef struct _SETUPDATA
{
  HFONT hTitleFont;

  WCHAR OwnerName[51];
  WCHAR OwnerOrganization[51];
  WCHAR ComputerName[MAX_COMPUTERNAME_LENGTH + 1];	/* max. 15 characters */
  WCHAR AdminPassword[15];				/* max. 14 characters */
  BOOL  UnattendSetup;
  BOOL  DisableVmwInst;

  SYSTEMTIME SystemTime;
  PTIMEZONE_ENTRY TimeZoneListHead;
  PTIMEZONE_ENTRY TimeZoneListTail;
  DWORD TimeZoneIndex;
  DWORD DisableAutoDaylightTimeSet;
  LCID LocaleID;
} SETUPDATA, *PSETUPDATA;


extern HINSTANCE hDllInstance;
extern HINF hSysSetupInf;
extern SETUPDATA SetupData;

/* wizard.c */
VOID InstallWizard (VOID);

/* EOF */
