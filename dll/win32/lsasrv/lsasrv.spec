
 @ stub LsaIAuditNotifyPackageLoad
 @ stub LsaIAuditSamEvent
 @ stub LsaIEnumerateSecrets
 @ stub LsaIFree_LSAI_PRIVATE_DATA #DATA
 @ stub LsaIFree_LSAI_SECRET_ENUM_BUFFER
 @ stub LsaIFree_LSAPR_ACCOUNT_ENUM_BUFFER
 @ stub LsaIFree_LSAPR_CR_CIPHER_VALUE
 @ stub LsaIFree_LSAPR_POLICY_INFORMATION
 @ stub LsaIFree_LSAPR_PRIVILEGE_ENUM_BUFFER
 @ stub LsaIFree_LSAPR_PRIVILEGE_SET
 @ stub LsaIFree_LSAPR_REFERENCED_DOMAIN_LIST
 @ stub LsaIFree_LSAPR_SR_SECURITY_DESCRIPTOR
 @ stub LsaIFree_LSAPR_TRANSLATED_NAMES
 @ stub LsaIFree_LSAPR_TRANSLATED_SIDS
 @ stub LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO
 @ stub LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER
 @ stub LsaIFree_LSAPR_TRUST_INFORMATION
 @ stub LsaIFree_LSAPR_UNICODE_STRING
 @ stub LsaIGetPrivateData
 @ stub LsaIGetSerialNumberPolicy
 @ stub LsaIGetSerialNumberPolicy2
 @ stub LsaIHealthCheck
 @ stub LsaIInitializeWellKnownSids
 @ stub LsaIOpenPolicyTrusted
 @ stub LsaIQueryInformationPolicyTrusted
 @ stub LsaISetPrivateData
 @ stub LsaISetSerialNumberPolicy
 @ stub LsaISetTimesSecret
 @ stub LsaISetupWasRun
 @ stub LsapAuOpenSam
 @ stdcall LsapInitLsa()
 @ stdcall LsarAddPrivilegesToAccount(ptr ptr)
 @ stdcall LsarClose(ptr)
 @ stdcall LsarCreateAccount(ptr ptr long ptr)
 @ stdcall LsarCreateSecret(ptr ptr long ptr)
 @ stdcall LsarCreateTrustedDomain(ptr ptr long ptr)
 @ stdcall LsarDelete(ptr)
 @ stdcall LsarEnumerateAccounts(ptr ptr ptr long)
 @ stdcall LsarEnumeratePrivileges(ptr ptr ptr long)
 @ stdcall LsarEnumeratePrivilegesAccount(ptr ptr)
 @ stdcall LsarEnumerateTrustedDomains(ptr ptr ptr long)
 @ stdcall LsarGetQuotasForAccount(ptr ptr)
 @ stdcall LsarGetSystemAccessAccount(ptr ptr)
 @ stdcall LsarLookupNames(ptr long ptr ptr ptr long ptr)
 @ stdcall LsarLookupPrivilegeDisplayName(ptr ptr long long ptr ptr)
 @ stdcall LsarLookupPrivilegeName(ptr ptr ptr)
 @ stdcall LsarLookupPrivilegeValue(ptr ptr ptr)
 @ stdcall LsarLookupSids(ptr ptr ptr ptr long ptr)
 @ stdcall LsarOpenAccount(ptr ptr long ptr)
 @ stdcall LsarOpenPolicy(ptr ptr long ptr)
 @ stdcall LsarOpenSecret(ptr ptr long ptr)
 @ stdcall LsarOpenTrustedDomain(ptr ptr long ptr)
 @ stdcall LsarQueryInfoTrustedDomain(ptr long ptr)
 @ stdcall LsarQueryInformationPolicy(ptr long ptr)
 @ stdcall LsarQuerySecret(ptr ptr ptr ptr ptr)
 @ stdcall LsarQuerySecurityObject(ptr long ptr)
 @ stdcall LsarRemovePrivilegesFromAccount(ptr long ptr)
 @ stdcall LsarSetInformationPolicy(ptr long ptr)
 @ stdcall LsarSetInformationTrustedDomain(ptr long ptr)
 @ stdcall LsarSetQuotasForAccount(ptr ptr)
 @ stdcall LsarSetSecret(ptr ptr ptr)
 @ stdcall LsarSetSecurityObject(ptr long ptr)
 @ stdcall LsarSetSystemAccessAccount(ptr long)
 @ stdcall ServiceInit()
