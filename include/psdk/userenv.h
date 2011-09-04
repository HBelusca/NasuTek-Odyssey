#ifndef _USERENV_H
#define _USERENV_H

#ifdef __cplusplus
extern "C" {
#endif

#define PI_NOUI (1)
#define PI_APPLYPOLICY (2)

#if (WINVER >= 0x0500)
#define RP_FORCE (1)
#endif

typedef struct _PROFILEINFOA
{
  DWORD dwSize;
  DWORD dwFlags;
  LPSTR lpUserName;
  LPSTR lpProfilePath;
  LPSTR lpDefaultPath;
  LPSTR lpServerName;
  LPSTR lpPolicyPath;
  HANDLE hProfile;
} PROFILEINFOA, *LPPROFILEINFOA;

typedef struct _PROFILEINFOW
{
  DWORD dwSize;
  DWORD dwFlags;
  LPWSTR lpUserName;
  LPWSTR lpProfilePath;
  LPWSTR lpDefaultPath;
  LPWSTR lpServerName;
  LPWSTR lpPolicyPath;
  HANDLE hProfile;
} PROFILEINFOW, *LPPROFILEINFOW;

/* begin private */
BOOL WINAPI InitializeProfiles (VOID);
BOOL WINAPI CreateUserProfileA (PSID, LPCSTR);
BOOL WINAPI CreateUserProfileW (PSID, LPCWSTR);
BOOL WINAPI AddDesktopItemA (BOOL, LPCSTR, LPCSTR, LPCSTR, INT, LPCSTR, WORD, INT);
BOOL WINAPI AddDesktopItemW (BOOL, LPCWSTR, LPCWSTR, LPCWSTR, INT, LPCWSTR, WORD, INT);
BOOL WINAPI DeleteDesktopItemA (BOOL, LPCSTR);
BOOL WINAPI DeleteDesktopItemW (BOOL, LPCWSTR);
BOOL WINAPI CreateGroupA (LPCSTR, BOOL);
BOOL WINAPI CreateGroupW (LPCWSTR, BOOL);
BOOL WINAPI DeleteGroupA (LPCSTR, BOOL);
BOOL WINAPI DeleteGroupW (LPCWSTR, BOOL);
BOOL WINAPI AddItemA (LPCSTR, BOOL, LPCSTR, LPCSTR, LPCSTR, INT, LPCSTR, WORD, INT);
BOOL WINAPI AddItemW (LPCWSTR, BOOL, LPCWSTR, LPCWSTR, LPCWSTR, INT, LPCWSTR, WORD, INT);
BOOL WINAPI DeleteItemA (LPCSTR, BOOL, LPCSTR, BOOL);
BOOL WINAPI DeleteItemW (LPCWSTR, BOOL, LPCWSTR, BOOL);
BOOL WINAPI CopyProfileDirectoryA(LPCSTR, LPCSTR, DWORD);
BOOL WINAPI CopyProfileDirectoryW(LPCWSTR, LPCWSTR, DWORD);
/* end private */
BOOL WINAPI LoadUserProfileA (HANDLE, LPPROFILEINFOA);
BOOL WINAPI LoadUserProfileW (HANDLE, LPPROFILEINFOW);
BOOL WINAPI UnloadUserProfile (HANDLE, HANDLE);

BOOL WINAPI GetAllUsersProfileDirectoryA (LPSTR, LPDWORD);
BOOL WINAPI GetAllUsersProfileDirectoryW (LPWSTR, LPDWORD);
BOOL WINAPI GetDefaultUserProfileDirectoryA (LPSTR, LPDWORD);
BOOL WINAPI GetDefaultUserProfileDirectoryW (LPWSTR, LPDWORD);
BOOL WINAPI GetProfilesDirectoryA(LPSTR, LPDWORD);
BOOL WINAPI GetProfilesDirectoryW(LPWSTR, LPDWORD);
BOOL WINAPI GetUserProfileDirectoryA(HANDLE, LPSTR, LPDWORD);
BOOL WINAPI GetUserProfileDirectoryW(HANDLE, LPWSTR, LPDWORD);

BOOL WINAPI CreateEnvironmentBlock(LPVOID*, HANDLE, BOOL);
BOOL WINAPI DestroyEnvironmentBlock(LPVOID);
#if (WINVER >= 0x0500)
BOOL WINAPI ExpandEnvironmentStringsForUserA (HANDLE, LPCSTR, LPSTR, DWORD);
BOOL WINAPI ExpandEnvironmentStringsForUserW (HANDLE, LPCWSTR, LPWSTR, DWORD);
#endif

HANDLE WINAPI EnterCriticalPolicySection (BOOL);
BOOL WINAPI LeaveCriticalPolicySection (HANDLE);
BOOL WINAPI RefreshPolicy (BOOL);
#if (WINVER >= 0x0500)
BOOL WINAPI RefreshPolicyEx (BOOL, DWORD);
#endif
BOOL WINAPI RegisterGPNotification (HANDLE, BOOL);
BOOL WINAPI UnregisterGPNotification (HANDLE);

#ifdef UNICODE
typedef PROFILEINFOW PROFILEINFO;
typedef LPPROFILEINFOW LPPROFILEINFO;
/* begin private */
#define CreateUserProfile  CreateUserProfileW
#define AddDesktopItem  AddDesktopItemW
#define DeleteDesktopItem  DeleteDesktopItemW
#define CreateGroup  CreateGroupW
#define DeleteGroup  DeleteGroupW
#define AddItem  AddItemW
#define DeleteItem  DeleteItemW
#define CopyProfileDirectory  CopyProfileDirectoryW
/* end private */
#define LoadUserProfile  LoadUserProfileW
#define GetAllUsersProfileDirectory  GetAllUsersProfileDirectoryW
#define GetDefaultUserProfileDirectory  GetDefaultUserProfileDirectoryW
#define GetProfilesDirectory  GetProfilesDirectoryW
#define GetUserProfileDirectory  GetUserProfileDirectoryW
#if (WINVER >= 0x0500)
#define ExpandEnvironmentStringsForUser ExpandEnvironmentStringsForUserW
#endif
#else
typedef PROFILEINFOA PROFILEINFO;
typedef LPPROFILEINFOA LPPROFILEINFO;
/* begin private */
#define CreateUserProfile  CreateUserProfileA
#define AddDesktopItem  AddDesktopItemA
#define DeleteDesktopItem  DeleteDesktopItemA
#define CreateGroup  CreateGroupA
#define DeleteGroup  DeleteGroupA
#define AddItem  AddItemA
#define DeleteItem  DeleteItemA
#define CopyProfileDirectory  CopyProfileDirectoryA
/* end private */
#define LoadUserProfile  LoadUserProfileA
#define GetAllUsersProfileDirectory  GetAllUsersProfileDirectoryA
#define GetDefaultUserProfileDirectory  GetDefaultUserProfileDirectoryA
#define GetProfilesDirectory  GetProfilesDirectoryA
#define GetUserProfileDirectory  GetUserProfileDirectoryA
#if (WINVER >= 0x0500)
#define ExpandEnvironmentStringsForUser ExpandEnvironmentStringsForUserA
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* _USERENV_H */
