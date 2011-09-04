/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey system libraries
 * FILE:        lib/msvcrt/misc/amsg.c
 * PURPOSE:     Print runtime error messages
 * PROGRAMER:   Ariadne
 * UPDATE HISTORY:
 *              28/12/98: Created
 */

#include <precomp.h>

static char *__rt_err_msg[] =
{
   "stack overflow",				/* _RT_STACK */
   "null pointer assignment",			/* _RT_NULLPTR */
   "floating point not loaded",			/* _RT_FLOAT */
   "integer divide by 0",			/* _RT_INTDIV */
   "not enough space for arguments",		/* _RT_SPACEARG */
   "not enough space for environment",		/* _RT_SPACEENV */
   "abnormal program termination",		/* _RT_ABORT */
   "not enough space for thread data",		/* _RT_THREAD */
   "unexpected multithread lock error",		/* _RT_LOCK */
   "unexpected heap error",			/* _RT_HEAP */
   "unable to open console device",		/* _RT_OPENCON */
   "non-continuable exception",			/* _RT_NONCONT */
   "invalid exception disposition",		/* _RT_INVALDISP */
   "not enough space for _onexit/atexit table",	/* _RT_ONEXIT */
   "pure virtual function call",		/* _RT_PUREVIRT */
   "not enough space for stdio initialization",	/* _RT_STDIOINIT */
   "not enough space for lowio initialization",	/* _RT_LOWIOINIT */
};


/*
 * @implemented
 */
int _aexit_rtn(int exitcode)
{
    _exit(exitcode);
    return 0;
}

/*
 * @implemented
 */
void _amsg_exit(int errnum)
{
    fprintf(stderr, "runtime error - %s\n", __rt_err_msg[errnum]);
    _aexit_rtn(-1);
}

