/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey Ancillary Function Driver DLL
 * FILE:        include/debug.h
 * PURPOSE:     Debugging support macros
 * DEFINES:     DBG     - Enable debug output
 *              NASSERT - Disable assertions
 */
#ifndef __DEBUG_H
#define __DEBUG_H

#define NORMAL_MASK    0x000000FF
#define SPECIAL_MASK   0xFFFFFF00
#define MIN_TRACE      0x00000001
#define MID_TRACE      0x00000002
#define MAX_TRACE      0x00000003

#define DEBUG_CHECK    0x00000100
#define DEBUG_ULTRA    0xFFFFFFFF

#ifdef ASSERT
#undef ASSERT
#endif

#if DBG

extern DWORD DebugTraceLevel;

#define AFD_DbgPrint(_t_, _x_) \
    if (((DebugTraceLevel & NORMAL_MASK) >= _t_) || \
        ((DebugTraceLevel & _t_) > NORMAL_MASK)) { \
        DbgPrint("(%hS:%d)(%hS) ", __FILE__, __LINE__, __FUNCTION__); \
		DbgPrint _x_; \
    }

#ifdef NASSERT
#define ASSERT(x)
#else /* NASSERT */
#define ASSERT(x) if (!(x)) { AFD_DbgPrint(MIN_TRACE, ("Assertion "#x" failed at %s:%d\n", __FILE__, __LINE__)); ExitProcess(0); }
#endif /* NASSERT */

#else /* DBG */

#define AFD_DbgPrint(_t_, _x_)

#define ASSERT_IRQL(x)
#define ASSERT(x)

#endif /* DBG */

#ifdef assert
#undef assert
#endif
#define assert(x) ASSERT(x)


#define UNIMPLEMENTED \
    AFD_DbgPrint(MIN_TRACE, ("is unimplemented, please try again later.\n"));

#define CHECKPOINT \
    AFD_DbgPrint(DEBUG_CHECK, ("\n"));

#define CP CHECKPOINT

#endif /* __DEBUG_H */

/* EOF */
