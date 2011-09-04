/*
 * Driver routines
 *
 * Copyright 2001 - Travis Michielsen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef __WINE_CRYPT_H
#define __WINE_CRYPT_H

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "wincrypt.h"

typedef struct tagPROVFUNCS
{
	BOOL (WINAPI *pCPAcquireContext)(HCRYPTPROV *phProv, LPSTR pszContainer, DWORD dwFlags, PVTableProvStruc pVTable);
	BOOL (WINAPI *pCPCreateHash)(HCRYPTPROV hProv, ALG_ID Algid, HCRYPTKEY hKey, DWORD dwFlags, HCRYPTHASH *phHash);
	BOOL (WINAPI *pCPDecrypt)(HCRYPTPROV hProv, HCRYPTKEY hKey, HCRYPTHASH hHash, BOOL Final, DWORD dwFlags, BYTE *pbData, DWORD *pdwDataLen);
	BOOL (WINAPI *pCPDeriveKey)(HCRYPTPROV hProv, ALG_ID     Algid, HCRYPTHASH hBaseData, DWORD dwFlags, HCRYPTKEY *phKey);
	BOOL (WINAPI *pCPDestroyHash)(HCRYPTPROV hProv, HCRYPTHASH hHash);
	BOOL (WINAPI *pCPDestroyKey)(HCRYPTPROV hProv, HCRYPTKEY hKey);
	BOOL (WINAPI *pCPDuplicateHash)(HCRYPTPROV hUID, HCRYPTHASH hHash, DWORD *pdwReserved, DWORD dwFlags, HCRYPTHASH *phHash);
	BOOL (WINAPI *pCPDuplicateKey)(HCRYPTPROV hUID, HCRYPTKEY hKey, DWORD *pdwReserved, DWORD dwFlags, HCRYPTKEY *phKey);
	BOOL (WINAPI *pCPEncrypt)(HCRYPTPROV hProv, HCRYPTKEY hKey, HCRYPTHASH hHash, BOOL Final, DWORD dwFlags, BYTE *pbData, DWORD *pdwDataLen, DWORD dwBufLen);
	BOOL (WINAPI *pCPExportKey)(HCRYPTPROV hProv, HCRYPTKEY hKey, HCRYPTKEY hPubKey, DWORD dwBlobType, DWORD dwFlags, BYTE *pbData, DWORD *pdwDataLen);
	BOOL (WINAPI *pCPGenKey)(HCRYPTPROV hProv, ALG_ID Algid, DWORD dwFlags, HCRYPTKEY *phKey);
	BOOL (WINAPI *pCPGenRandom)(HCRYPTPROV hProv, DWORD dwLen, BYTE *pbBuffer);
	BOOL (WINAPI *pCPGetHashParam)(HCRYPTPROV hProv, HCRYPTHASH hHash, DWORD dwParam, BYTE *pbData, DWORD *pdwDataLen, DWORD dwFlags);
	BOOL (WINAPI *pCPGetKeyParam)(HCRYPTPROV hProv, HCRYPTKEY hKey, DWORD dwParam, BYTE *pbData, DWORD *pdwDataLen, DWORD dwFlags);
	BOOL (WINAPI *pCPGetProvParam)(HCRYPTPROV hProv, DWORD dwParam, BYTE *pbData, DWORD *pdwDataLen, DWORD dwFlags);
	BOOL (WINAPI *pCPGetUserKey)(HCRYPTPROV hProv, DWORD dwKeySpec, HCRYPTKEY *phUserKey);
	BOOL (WINAPI *pCPHashData)(HCRYPTPROV hProv, HCRYPTHASH hHash, CONST BYTE *pbData, DWORD dwDataLen, DWORD dwFlags);
	BOOL (WINAPI *pCPHashSessionKey)(HCRYPTPROV hProv, HCRYPTHASH hHash, HCRYPTKEY hKey, DWORD dwFlags);
	BOOL (WINAPI *pCPImportKey)(HCRYPTPROV hProv, CONST BYTE *pbData, DWORD dwDataLen, HCRYPTKEY hPubKey, DWORD dwFlags, HCRYPTKEY *phKey);
	BOOL (WINAPI *pCPReleaseContext)(HCRYPTPROV hProv, DWORD dwFlags);
	BOOL (WINAPI *pCPSetHashParam)(HCRYPTPROV hProv, HCRYPTHASH hHash, DWORD dwParam, CONST BYTE *pbData, DWORD dwFlags);
	BOOL (WINAPI *pCPSetKeyParam)(HCRYPTPROV hProv, HCRYPTKEY hKey, DWORD dwParam, CONST BYTE *pbData, DWORD dwFlags);
	BOOL (WINAPI *pCPSetProvParam)(HCRYPTPROV hProv, DWORD dwParam, CONST BYTE *pbData, DWORD dwFlags);
	BOOL (WINAPI *pCPSignHash)(HCRYPTPROV hProv, HCRYPTHASH hHash, DWORD dwKeySpec, LPCWSTR sDescription, DWORD dwFlags, BYTE *pbSignature, DWORD *pdwSigLen);
	BOOL (WINAPI *pCPVerifySignature)(HCRYPTPROV hProv, HCRYPTHASH hHash, CONST BYTE *pbSignature, DWORD dwSigLen, HCRYPTKEY hPubKey, LPCWSTR sDescription, DWORD dwFlags);
} PROVFUNCS, *PPROVFUNCS;

#define MAGIC_CRYPTPROV 0xA39E741F

typedef struct tagCRYPTPROV
{
	DWORD dwMagic;
	UINT refcount;
	HMODULE hModule;
	PPROVFUNCS pFuncs;
        HCRYPTPROV hPrivate;  /*CSP's handle - Should not be given to application under any circumstances!*/
	PVTableProvStruc pVTable;
} CRYPTPROV, *PCRYPTPROV;

typedef struct tagCRYPTKEY
{
	PCRYPTPROV pProvider;
        HCRYPTKEY hPrivate;    /*CSP's handle - Should not be given to application under any circumstances!*/
} CRYPTKEY, *PCRYPTKEY;

typedef struct tagCRYPTHASH
{
	PCRYPTPROV pProvider;
        HCRYPTHASH hPrivate;    /*CSP's handle - Should not be given to application under any circumstances!*/
} CRYPTHASH, *PCRYPTHASH;

#define MAXPROVTYPES 999

extern unsigned char *CRYPT_DEShash( unsigned char *dst, const unsigned char *key,
                                     const unsigned char *src );
extern unsigned char *CRYPT_DESunhash( unsigned char *dst, const unsigned char *key,
                                       const unsigned char *src );

void byteReverse(unsigned char *buf, unsigned longs);
struct ustring {
    DWORD Length;
    DWORD MaximumLength;
    unsigned char *Buffer;
};

typedef struct {
    unsigned int buf[4];
    unsigned int i[2];
    unsigned char in[64];
    unsigned char digest[16];
} MD4_CTX;

typedef struct tag_arc4_info {
    unsigned char state[256];
    unsigned char x, y;
} arc4_info;

VOID WINAPI MD4Init( MD4_CTX *ctx );
VOID WINAPI MD4Update( MD4_CTX *ctx, const unsigned char *buf, unsigned int len );
VOID WINAPI MD4Final(MD4_CTX *ctx);
void arc4_init(arc4_info *a4i, const BYTE *key, unsigned int keyLen);
void arc4_ProcessString(arc4_info *a4i, BYTE *inoutString, unsigned int length);
NTSTATUS WINAPI SystemFunction032(struct ustring *data, struct ustring *key);


#endif /* __WINE_CRYPT_H_ */
