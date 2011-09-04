/*
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          Odyssey kernel
 * PURPOSE:          Native driver for dxg implementation
 * FILE:             drivers/directx/dxg/main.c
 * PROGRAMER:        Magnus olsen (magnus@greatlord.com)
 * REVISION HISTORY:
 *       15/10-2007   Magnus Olsen
 */

#include <dxg_int.h>
#include "dxg_driver.h"

LONG gcDummyPageRefCnt = 0;
HSEMAPHORE ghsemDummyPage = NULL;
VOID *gpDummyPage = NULL;
PEPROCESS gpepSession = NULL;
PLARGE_INTEGER gpLockShortDelay = NULL;

PDRVFN gpEngFuncs;
const ULONG gcDxgFuncs = DXG_INDEX_DxDdIoctl + 1;


NTSTATUS NTAPI
DriverEntry(IN PVOID Context1,
            IN PVOID Context2)
{
    return 0;
}

NTSTATUS
APIENTRY
DxDdStartupDxGraphics (ULONG SizeEngDrv,
                       PDRVENABLEDATA pDxEngDrv,
                       ULONG SizeDxgDrv,
                       PDRVENABLEDATA pDxgDrv,
                       PULONG DirectDrawContext,
                       PEPROCESS Proc )
{

    PDRVFN drv_func;
    UINT i;

    /* Test see if the data is vaild we got from win32k.sys */
    if ((SizeEngDrv != sizeof(DRVENABLEDATA)) ||
        (SizeDxgDrv != sizeof(DRVENABLEDATA)))
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    /* rest static value */
    gpDummyPage = NULL;
    gcDummyPageRefCnt = 0;
    ghsemDummyPage = NULL;

    /*
     * Setup internal driver functions list we got from dxg driver functions list
     */
    pDxgDrv->iDriverVersion = 0x80000; /* Note 12/1-2004 : DirectX 8 ? */
    pDxgDrv->c = gcDxgFuncs;
    pDxgDrv->pdrvfn = gaDxgFuncs;

    /* check how many driver functions and fail if the value does not match */
    if (pDxEngDrv->c !=  DXENG_INDEX_DxEngLoadImage + 1)
    {
        return STATUS_INTERNAL_ERROR;
    }

    /*
     * Check if all drv functions are sorted right
     * and if it really are exported
     */

    for (i=1 ; i < DXENG_INDEX_DxEngLoadImage + 1; i++)
    {
        drv_func = &pDxEngDrv->pdrvfn[i];

        if ((drv_func->iFunc != i) ||
            (drv_func->pfn == NULL))
        {
            return STATUS_INTERNAL_ERROR;
        }
    }

    gpEngFuncs = pDxEngDrv->pdrvfn;

    /* Note 12/1-2004 : Why is this set to 0x618 */
    *DirectDrawContext = 0x618;

    if (DdHmgCreate())
    {
        ghsemDummyPage = EngCreateSemaphore();

        if (ghsemDummyPage)
        {
            gpepSession = Proc;
            return STATUS_SUCCESS;
        }
    }

    DdHmgDestroy();

    if (ghsemDummyPage)
    {
        EngDeleteSemaphore(ghsemDummyPage);
        ghsemDummyPage = 0;
    }

    return STATUS_NO_MEMORY;
}

NTSTATUS
APIENTRY
DxDdCleanupDxGraphics(VOID)
{
    DdHmgDestroy();

    if (ghsemDummyPage != 0 )
    {
        if (gpDummyPage != 0 )
        {
            ExFreePoolWithTag(gpDummyPage,0);
            gpDummyPage = NULL;
            gcDummyPageRefCnt = 0;
        }
        EngDeleteSemaphore(ghsemDummyPage);
        ghsemDummyPage = 0;
    }

    return 0;
}
