/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey WinSock 2 Helper DLL for TCP/IP
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

#define DEBUG_ULTRA    0xFFFFFFFF

#if DBG

extern DWORD DebugTraceLevel;

#define WSH_DbgPrint(_t_, _x_) \
    if (((DebugTraceLevel & NORMAL_MASK) >= _t_) || \
        ((DebugTraceLevel & _t_) > NORMAL_MASK)) { \
        DbgPrint("(%hS:%d)(%hS) ", __FILE__, __LINE__, __FUNCTION__); \
		DbgPrint _x_; \
    }

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef NASSERT
#define ASSERT(x)
#else /* NASSERT */
#define ASSERT(x) if (!(x)) { WSH_DbgPrint(MIN_TRACE, ("Assertion "#x" failed at %s:%d\n", __FILE__, __LINE__)); ExitProcess(0); }
#endif /* NASSERT */

#else /* DBG */

#define WSH_DbgPrint(_t_, _x_)

#undef ASSERT
#define ASSERT(x)

#endif /* DBG */


#undef assert
#define assert(x) ASSERT(x)
#define assert_irql(x) ASSERT_IRQL(x)


#define UNIMPLEMENTED \
    WSH_DbgPrint(MIN_TRACE, ("(%s:%d)(%s) is unimplemented, \
        please try again later.\n", __FILE__, __LINE__, __FUNCTION__));

#define CHECKPOINT \
    WSH_DbgPrint(MIN_TRACE, ("\n"));

#define CP CHECKPOINT

#endif /* __DEBUG_H */

/* EOF */
