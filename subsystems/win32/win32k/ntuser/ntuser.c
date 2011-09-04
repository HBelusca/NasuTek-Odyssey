/*
 *  COPYRIGHT:        See COPYING in the top level directory
 *  PROJECT:          Odyssey kernel
 *  PURPOSE:          ntuser init. and main funcs.
 *  FILE:             subsystems/win32/win32k/ntuser/ntuser.c
 *  REVISION HISTORY:
 *       16 July 2005   Created (hardon)
 */

/* INCLUDES ******************************************************************/

#include <win32k.h>

DBG_DEFAULT_CHANNEL(UserMisc);

BOOL InitSysParams();

/* GLOBALS *******************************************************************/

PTHREADINFO gptiCurrent = NULL;
ERESOURCE UserLock;
ATOM AtomMessage; // Window Message atom.
ATOM AtomWndObj;  // Window Object atom.
ATOM AtomLayer;   // Window Layer atom.
ATOM AtomFlashWndState; // Window Flash State atom.
BOOL gbInitialized;
HINSTANCE hModClient = NULL;
BOOL ClientPfnInit = FALSE;

/* PRIVATE FUNCTIONS *********************************************************/

static
NTSTATUS FASTCALL
InitUserAtoms(VOID)
{

  gpsi->atomSysClass[ICLS_MENU]      = 32768;
  gpsi->atomSysClass[ICLS_DESKTOP]   = 32769;
  gpsi->atomSysClass[ICLS_DIALOG]    = 32770;
  gpsi->atomSysClass[ICLS_SWITCH]    = 32771;
  gpsi->atomSysClass[ICLS_ICONTITLE] = 32772;
  gpsi->atomSysClass[ICLS_TOOLTIPS]  = 32774;

  /* System Message Atom */
  AtomMessage = IntAddGlobalAtom(L"Message", TRUE);
  gpsi->atomSysClass[ICLS_HWNDMESSAGE] = AtomMessage;

  /* System Context Help Id Atom */
  gpsi->atomContextHelpIdProp = IntAddGlobalAtom(L"SysCH", TRUE);

  AtomWndObj = IntAddGlobalAtom(L"SysWNDO", TRUE);
  AtomLayer = IntAddGlobalAtom(L"SysLayer", TRUE);
  AtomFlashWndState = IntAddGlobalAtom(L"FlashWState", TRUE);

  return STATUS_SUCCESS;
}

/* FUNCTIONS *****************************************************************/

INIT_FUNCTION
NTSTATUS
NTAPI
InitUserImpl(VOID)
{
   NTSTATUS Status;

   ExInitializeResourceLite(&UserLock);

   if (!UserCreateHandleTable())
   {
      ERR("Failed creating handle table\n");
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   Status = InitSessionImpl();
   if (!NT_SUCCESS(Status))
   {
      ERR("Error init session impl.\n");
      return Status;
   }

   InitUserAtoms();

   InitSysParams();

   return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
InitVideo();

NTSTATUS
NTAPI
UserInitialize(
  HANDLE  hPowerRequestEvent,
  HANDLE  hMediaRequestEvent)
{
    static const DWORD wPattern55AA[] = /* 32 bit aligned */
    { 0x55555555, 0xaaaaaaaa, 0x55555555, 0xaaaaaaaa,
      0x55555555, 0xaaaaaaaa, 0x55555555, 0xaaaaaaaa };
    HBITMAP hPattern55AABitmap = NULL;
    NTSTATUS Status;

// Set W32PF_Flags |= (W32PF_READSCREENACCESSGRANTED | W32PF_IOWINSTA)
// Create Object Directory,,, Looks like create workstation. "\\Windows\\WindowStations"
// Create Event for Diconnect Desktop.

    /* Initialize Video. */
    Status = InitVideo();
    if (!NT_SUCCESS(Status)) return Status;

// {
//     DrvInitConsole.
//     DrvChangeDisplaySettings.
//     Update Shared Device Caps.
//     Initialize User Screen.
// }
// Create ThreadInfo for this Thread!
// {

    GetW32ThreadInfo();

//    Callback to User32 Client Thread Setup

    co_IntClientThreadSetup();

// }
// Set Global SERVERINFO Error flags.
// Load Resources.

    NtUserUpdatePerUserSystemParameters(0, TRUE);

    CsrInit();

    if (gpsi->hbrGray == NULL)
    {
       hPattern55AABitmap = GreCreateBitmap(8, 8, 1, 1, (LPBYTE)wPattern55AA);
       gpsi->hbrGray = IntGdiCreatePatternBrush(hPattern55AABitmap);
       GreDeleteObject(hPattern55AABitmap);
       GreSetBrushOwner(gpsi->hbrGray, GDI_OBJ_HMGR_PUBLIC);
    }

    return STATUS_SUCCESS;
}

/*
    Called from win32csr.
 */
NTSTATUS
APIENTRY
NtUserInitialize(
  DWORD   dwWinVersion,
  HANDLE  hPowerRequestEvent,
  HANDLE  hMediaRequestEvent)
{
    NTSTATUS Status;

    ERR("Enter NtUserInitialize(%lx, %p, %p)\n",
            dwWinVersion, hPowerRequestEvent, hMediaRequestEvent);

    /* Check the Windows version */
    if (dwWinVersion != 0)
    {
        return STATUS_UNSUCCESSFUL;
    }

    /* Acquire exclusive lock */
    UserEnterExclusive();

    /* Check if we are already initialized */
    if (gbInitialized)
    {
        UserLeave();
        return STATUS_UNSUCCESSFUL;
    }

// Initialize Power Request List.
// Initialize Media Change.
// InitializeGreCSRSS();
// {
//    Startup DxGraphics.
//    calls ** IntGdiGetLanguageID() and sets it **.
//    Enables Fonts drivers, Initialize Font table & Stock Fonts.
// }

    /* Initialize USER */
    Status = UserInitialize(hPowerRequestEvent, hMediaRequestEvent);

    /* Set us as initialized */
    gbInitialized = TRUE;

    /* Return */
    UserLeave();
    return Status;
}


/*
RETURN
   True if current thread owns the lock (possibly shared)
*/
BOOL FASTCALL UserIsEntered(VOID)
{
   return ExIsResourceAcquiredExclusiveLite(&UserLock)
      || ExIsResourceAcquiredSharedLite(&UserLock);
}

BOOL FASTCALL UserIsEnteredExclusive(VOID)
{
   return ExIsResourceAcquiredExclusiveLite(&UserLock);
}

VOID FASTCALL CleanupUserImpl(VOID)
{
   ExDeleteResourceLite(&UserLock);
}

VOID FASTCALL UserEnterShared(VOID)
{
   KeEnterCriticalRegion();
   ExAcquireResourceSharedLite(&UserLock, TRUE);
}

VOID FASTCALL UserEnterExclusive(VOID)
{
   ASSERT_NOGDILOCKS();
   KeEnterCriticalRegion();
   ExAcquireResourceExclusiveLite(&UserLock, TRUE);
   gptiCurrent = PsGetCurrentThreadWin32Thread();
}

VOID FASTCALL UserLeave(VOID)
{
   ASSERT_NOGDILOCKS();
   ExReleaseResourceLite(&UserLock);
   KeLeaveCriticalRegion();
}
