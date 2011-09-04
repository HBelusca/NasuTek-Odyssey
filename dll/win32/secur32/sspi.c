#include <precomp.h>

#define NDEBUG
#include <debug.h>

SECURITY_STATUS WINAPI ApplyControlTokenW(PCtxtHandle Handle, PSecBufferDesc Buffer);
SECURITY_STATUS WINAPI ApplyControlTokenA(PCtxtHandle Handle, PSecBufferDesc Buffer);

static SecurityFunctionTableA securityFunctionTableA =
{
    SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
    EnumerateSecurityPackagesA,
    QueryCredentialsAttributesA,
    AcquireCredentialsHandleA,
    FreeCredentialsHandle,
    NULL, /* Reserved2 */
    InitializeSecurityContextA,
    AcceptSecurityContext,
    CompleteAuthToken,
    DeleteSecurityContext,
    ApplyControlTokenA,
    QueryContextAttributesA,
    ImpersonateSecurityContext,
    RevertSecurityContext,
    MakeSignature,
    VerifySignature,
    FreeContextBuffer,
    QuerySecurityPackageInfoA,
    EncryptMessage, /* Reserved3 */
    DecryptMessage, /* Reserved4 */
    ExportSecurityContext,
    ImportSecurityContextA,
    AddCredentialsA,
    NULL, /* Reserved8 */
    QuerySecurityContextToken,
    EncryptMessage,
    DecryptMessage,
    NULL
};

static SecurityFunctionTableW securityFunctionTableW =
{
    SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
    EnumerateSecurityPackagesW,
    QueryCredentialsAttributesW,
    AcquireCredentialsHandleW,
    FreeCredentialsHandle,
    NULL, /* Reserved2 */
    InitializeSecurityContextW,
    AcceptSecurityContext,
    CompleteAuthToken,
    DeleteSecurityContext,
    ApplyControlTokenW,
    QueryContextAttributesW,
    ImpersonateSecurityContext,
    RevertSecurityContext,
    MakeSignature,
    VerifySignature,
    FreeContextBuffer,
    QuerySecurityPackageInfoW,
    EncryptMessage, /* Reserved3 */
    DecryptMessage, /* Reserved4 */
    ExportSecurityContext,
    ImportSecurityContextW,
    AddCredentialsW,
    NULL, /* Reserved8 */
    QuerySecurityContextToken,
    EncryptMessage,
    DecryptMessage,
    NULL
};

SECURITY_STATUS
WINAPI
EnumerateSecurityPackagesW (
	PULONG pcPackages,
	PSecPkgInfoW* ppPackageInfo
	)
{
    SECURITY_STATUS ret = SEC_E_OK;

    *pcPackages = 0;
    *ppPackageInfo = NULL;
    /*
    if (packageTable)
    {

    }
    */

    UNIMPLEMENTED;
    return ret;
}

SECURITY_STATUS
WINAPI
EnumerateSecurityPackagesA(
	PULONG pcPackages,
	PSecPkgInfoA* ppPackageInfo
	)
{
    SECURITY_STATUS ret;
    PSecPkgInfoW info = NULL;

    ret = EnumerateSecurityPackagesW(pcPackages, &info);
    if (ret == SEC_E_OK && *pcPackages && info)
    {
        /* *ppPackageInfo = thunk_PSecPkgInfoWToA(*pcPackages, info);
        if (*pcPackages && !*ppPackageInfo)
        {
            *pcPackages = 0;
            ret = SEC_E_INSUFFICIENT_MEMORY;
        } */
        FreeContextBuffer(info);
    }
    *ppPackageInfo = NULL;
	UNIMPLEMENTED;

    return ret;
}

SECURITY_STATUS
WINAPI
FreeContextBuffer (
	PVOID pvoid
	)
{
    HeapFree(GetProcessHeap(), 0, pvoid);
    return SEC_E_OK;
}

SECURITY_STATUS
WINAPI
FreeCredentialsHandle(PCredHandle Handle)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
DeleteSecurityContext(PCtxtHandle Handle)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

PSecurityFunctionTableW
WINAPI
InitSecurityInterfaceW(VOID)
{
    DPRINT("InitSecurityInterfaceW() called\n");
    return &securityFunctionTableW;
}

SECURITY_STATUS
WINAPI
EncryptMessage(PCtxtHandle Handle,
               ULONG Foo,
               PSecBufferDesc Buffer,
               ULONG Bar)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
DecryptMessage(PCtxtHandle Handle,
               PSecBufferDesc Buffer,
               ULONG Foo,
               PULONG Bar)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
ApplyControlTokenW(PCtxtHandle Handle,
                  PSecBufferDesc Buffer)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
ApplyControlTokenA(PCtxtHandle Handle,
                  PSecBufferDesc Buffer)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
CompleteAuthToken(PCtxtHandle Handle,
                  PSecBufferDesc Buffer)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
QueryContextAttributesA(PCtxtHandle Handle,
                        ULONG Foo,
                        PVOID Bar)
{
	UNIMPLEMENTED;

	if (Handle)
	{
		Bar = NULL;
		return ERROR_CALL_NOT_IMPLEMENTED;
	}

	return SEC_E_INVALID_HANDLE;
}

SECURITY_STATUS
WINAPI
QueryContextAttributesW(PCtxtHandle Handle,
                        ULONG Foo,
                        PVOID Bar)
{
	UNIMPLEMENTED;

	if (Handle)
	{
		Bar = NULL;
		return ERROR_CALL_NOT_IMPLEMENTED;
	}

	return SEC_E_INVALID_HANDLE;
}

SECURITY_STATUS
WINAPI
AcquireCredentialsHandleA (
    SEC_CHAR* pszPrincipal,
    SEC_CHAR* pszPackage,
    ULONG fUsage,
    PLUID pID,
    PVOID pAuth,
    SEC_GET_KEY_FN pGetKeyFn,
    PVOID pvGetKeyArgument,
    PCredHandle phCred,
    PTimeStamp pExpires
    )
{
	UNIMPLEMENTED;

	if (pszPackage)
	{
		phCred = NULL;
		pExpires = NULL;
		return ERROR_CALL_NOT_IMPLEMENTED;
	}

	return SEC_E_SECPKG_NOT_FOUND;
}

SECURITY_STATUS
WINAPI
AcquireCredentialsHandleW (
    SEC_WCHAR* pszPrincipal,
    SEC_WCHAR* pszPackage,
    ULONG fUsage,
    PLUID pID,
    PVOID pAuth,
    SEC_GET_KEY_FN pGetKeyFn,
    PVOID pvGetKeyArgument,
    PCredHandle phCred,
    PTimeStamp pExpires
    )
{
	UNIMPLEMENTED;

	if (pszPackage)
	{
		phCred = NULL;
		pExpires = NULL;
		return ERROR_CALL_NOT_IMPLEMENTED;
	}

	return SEC_E_SECPKG_NOT_FOUND;
}

SECURITY_STATUS
WINAPI
InitializeSecurityContextW (
    PCredHandle phCred,
    PCtxtHandle phContext,
    SEC_WCHAR* pszTarget,
    ULONG fContextReq,
    ULONG Reserved,
    ULONG TargetData,
    PSecBufferDesc pInput,
    ULONG Reserved2,
    PCtxtHandle phNewContext,
    PSecBufferDesc pOut,
    PULONG pfContextAttributes,
    PTimeStamp pExpires
    )
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
InitializeSecurityContextA (
    PCredHandle phCred,
    PCtxtHandle phContext,
    SEC_CHAR* pszTarget,
    ULONG fContextReq,
    ULONG Reserved,
    ULONG TargetData,
    PSecBufferDesc pInput,
    ULONG Reserved2,
    PCtxtHandle phNewContext,
    PSecBufferDesc pOut,
    PULONG pfContextAttributes,
    PTimeStamp pExpires
    )
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}


SECURITY_STATUS
SEC_ENTRY
MakeSignature(
    PCtxtHandle phContext,
    ULONG fQOP,
    PSecBufferDesc pMessage,
    ULONG MessageSeqNo
    )
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}


SECURITY_STATUS
SEC_ENTRY
VerifySignature(
    PCtxtHandle phContext,
    PSecBufferDesc pMessage,
    ULONG MessageSeqNo,
    PULONG pfQOP
    )
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
SEC_ENTRY
QuerySecurityPackageInfoA(
    SEC_CHAR* pszPackageName,
    PSecPkgInfoA* ppPackageInfo
)
{
	UNIMPLEMENTED;

	if (pszPackageName)
	{
		*ppPackageInfo = NULL;
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
	return SEC_E_SECPKG_NOT_FOUND;
}

SECURITY_STATUS
SEC_ENTRY
QuerySecurityPackageInfoW(
    SEC_WCHAR* pszPackageName,
    PSecPkgInfoW* ppPackageInfo
)
{
	UNIMPLEMENTED;

	if (pszPackageName)
	{
		*ppPackageInfo = NULL;
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
	return SEC_E_SECPKG_NOT_FOUND;
}

SECURITY_STATUS
WINAPI
AcceptSecurityContext(
    PCredHandle phCredential,
    PCtxtHandle phContext,
	PSecBufferDesc pInput,
    ULONG fContextReq,
	ULONG TargetDataRep,
	PCtxtHandle phNewContext,
    PSecBufferDesc pOutput,
	ULONG *pfContextAttr,
	PTimeStamp ptsExpiry
)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
AddCredentialsA(
	PCredHandle hCredentials,
	SEC_CHAR *pszPrincipal,
	SEC_CHAR *pszPackage,
	ULONG fCredentialUse,
	LPVOID pAuthData,
	SEC_GET_KEY_FN pGetKeyFn,
	LPVOID pvGetKeyArgument,
	PTimeStamp ptsExpiry
)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
AddCredentialsW(
	PCredHandle hCredentials,
	SEC_WCHAR *pszPrincipal,
	SEC_WCHAR *pszPackage,
	ULONG fCredentialUse,
	LPVOID pAuthData,
	SEC_GET_KEY_FN pGetKeyFn,
	LPVOID pvGetKeyArgument,
	PTimeStamp ptsExpiry
)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
ExportSecurityContext(
	PCtxtHandle phContext,
	ULONG fFlags,
	PSecBuffer pPackedContext,
	LPVOID *pToken
)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
ImpersonateSecurityContext(
	PCtxtHandle phContext
)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
ImportSecurityContextA(
	SEC_CHAR *pszPackage,
	PSecBuffer pPackedContext,
	LPVOID Token,
	PCtxtHandle phContext
)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
ImportSecurityContextW(
	SEC_WCHAR *pszPackage,
	PSecBuffer pPackedContext,
	LPVOID Token,
	PCtxtHandle phContext
)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
QueryCredentialsAttributesA(
	PCredHandle phCredential,
	ULONG ulAttribute,
	LPVOID pBuffer
)
{
	UNIMPLEMENTED;

	if (phCredential)
	{
		pBuffer = NULL;
		return ERROR_CALL_NOT_IMPLEMENTED;
	}

	return SEC_E_INVALID_HANDLE;
}

SECURITY_STATUS
WINAPI
QueryCredentialsAttributesW(
	PCredHandle phCredential,
	ULONG ulAttribute,
	LPVOID pBuffer
)
{
	UNIMPLEMENTED;

	if (phCredential)
	{
		pBuffer = NULL;
		return ERROR_CALL_NOT_IMPLEMENTED;
	}

	return SEC_E_INVALID_HANDLE;
}

SECURITY_STATUS
WINAPI
QuerySecurityContextToken(
	PCtxtHandle phContext,
	PHANDLE phToken
)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

SECURITY_STATUS
WINAPI
RevertSecurityContext(
	PCtxtHandle phContext
)
{
	UNIMPLEMENTED;
	return ERROR_CALL_NOT_IMPLEMENTED;
}

PSecurityFunctionTableA
WINAPI
InitSecurityInterfaceA(VOID)
{
    DPRINT("InitSecurityInterfaceA() called\n");
    return &securityFunctionTableA;
}

BOOLEAN
WINAPI
TranslateNameA(
	LPCSTR lpAccountName,
	EXTENDED_NAME_FORMAT AccountNameFormat,
	EXTENDED_NAME_FORMAT DesiredNameFormat,
	LPSTR lpTranslatedName,
	PULONG nSize
)
{
	UNIMPLEMENTED;
	return FALSE;
}

BOOLEAN
WINAPI
TranslateNameW(
	LPCWSTR lpAccountName,
	EXTENDED_NAME_FORMAT AccountNameFormat,
	EXTENDED_NAME_FORMAT DesiredNameFormat,
	LPWSTR lpTranslatedName,
	PULONG nSize
)
{
	UNIMPLEMENTED;
	return FALSE;
}
