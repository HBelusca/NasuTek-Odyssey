/* $Id: user.c 52859 2011-07-25 05:54:37Z dgorbachev $
 *
 * subsystems/win32/csrss/csrsrv/api/user.c
 *
 * User functions
 *
 * Odyssey Operating System
 *
 * PROGRAMMER: Eric Kohl
 */

/* INCLUDES ******************************************************************/

#include <srv.h>

#define NDEBUG
#include <debug.h>

/* GLOBALS *******************************************************************/

static BOOLEAN ServicesProcessIdValid = FALSE;
static ULONG_PTR ServicesProcessId;


/* FUNCTIONS *****************************************************************/

CSR_API(CsrRegisterServicesProcess)
{
  Request->Header.u1.s1.TotalLength = sizeof(CSR_API_MESSAGE);
  Request->Header.u1.s1.DataLength = sizeof(CSR_API_MESSAGE) - sizeof(PORT_MESSAGE);

  if (ServicesProcessIdValid == TRUE)
    {
      /* Only accept a single call */
      return STATUS_INVALID_PARAMETER;
    }
  else
    {
      ServicesProcessId = (ULONG_PTR)Request->Data.RegisterServicesProcessRequest.ProcessId;
      ServicesProcessIdValid = TRUE;
      return STATUS_SUCCESS;
    }
}

/* EOF */
