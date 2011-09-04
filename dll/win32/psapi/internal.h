/* $Id: internal.h 21298 2006-03-12 00:08:41Z jimtabor $
*/
/*
 * epsapi.h
 *
 * Process Status Helper API, native interface
 *
 * This file is part of the Odyssey Operating System.
 *
 * Contributors:
 *  Created by KJK::Hyperion <noog@libero.it>
 *
 *  THIS SOFTWARE IS NOT COPYRIGHTED
 *
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef __INTERNAL_PSAPI_H_INCLUDED__
#define __INTERNAL_PSAPI_H_INCLUDED__

#ifndef SetLastErrorByStatus
#define SetLastErrorByStatus(__S__) \
 ((void)SetLastError(RtlNtStatusToDosError(__S__)))
#endif /* SetLastErrorByStatus */

#endif /* __INTERNAL_PSAPI_H_INCLUDED__ */

/* EOF */
