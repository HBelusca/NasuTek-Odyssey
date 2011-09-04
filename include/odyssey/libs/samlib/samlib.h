/* $Id: samlib.h 45020 2010-01-09 22:43:16Z ekohl $
*/
/*
 * samlib.h
 *
 * Security Account Manager API, native interface
 *
 * This file is part of the Odyssey Operating System.
 *
 * Contributors:
 *  Created by Eric Kohl
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

#ifndef __SAMLIB_H_INCLUDED__
#define __SAMLIB_H_INCLUDED__


BOOL WINAPI
SamInitializeSAM (VOID);

BOOL WINAPI
SamGetDomainSid (PSID *Sid);

BOOL WINAPI
SamSetDomainSid (PSID Sid);

BOOL WINAPI
SamCreateUser (PWSTR UserName,
	       PWSTR UserPassword,
	       PSID UserSid);

BOOL WINAPI
SamCheckUserPassword (PWSTR UserName,
		      PWSTR UserPassword);

BOOL WINAPI
SamGetUserSid (PWSTR UserName,
	       PSID *Sid);

#endif /* __SAMLIB_H_INCLUDED__ */

/* EOF */
