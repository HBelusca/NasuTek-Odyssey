/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
**
** http://oss.sgi.com/projects/FreeB
**
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
**
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
**
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
**
** $Date: 2008-11-21 04:20:48 -0800 (Fri, 21 Nov 2008) $ $Revision: 1.1 $
** $Header: /cygdrive/c/RCVS/CVS/Odyssey/odyssey/lib/glu32/libutil/error.c,v 1.1 2004/02/02 16:39:15 navaraf Exp $
*/

#include "gluos.h"
#include "gluint.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>

const GLubyte* GLAPIENTRY
gluErrorString(GLenum errorCode)
{
    switch (errorCode)
    {
    case GL_NO_ERROR:
        return (const GLubyte *) "no error";
    case GL_INVALID_ENUM:
        return (const GLubyte *) "invalid enumerant";
    case GL_INVALID_VALUE:
        return (const GLubyte *) "invalid value";
    case GL_INVALID_OPERATION:
        return (const GLubyte *) "invalid operation";
    case GL_STACK_OVERFLOW:
        return (const GLubyte *) "stack overflow";
    case GL_STACK_UNDERFLOW:
        return (const GLubyte *) "stack underflow";
    case GL_OUT_OF_MEMORY:
        return (const GLubyte *) "out of memory";
    case GL_TABLE_TOO_LARGE:
        return (const GLubyte *) "table too large";
    case GLU_INVALID_ENUM:
        return (const GLubyte *) "invalid enumerant";
    case GLU_INVALID_VALUE:
        return (const GLubyte *) "invalid value";
    case GLU_OUT_OF_MEMORY:
        return (const GLubyte *) "out of memory";
    case GLU_INCOMPATIBLE_GL_VERSION:
        return (const GLubyte *) "incompatible gl version";
    case GLU_INVALID_OPERATION:
        return (const GLubyte *) "invalid operation";
    }

    if ((errorCode >= GLU_NURBS_ERROR1) && (errorCode <= GLU_NURBS_ERROR37)) {
	return (const GLubyte *) __gluNURBSErrorString(errorCode - (GLU_NURBS_ERROR1 - 1));
    }
    if ((errorCode >= GLU_TESS_ERROR1) && (errorCode <= GLU_TESS_ERROR6)) {
	return (const GLubyte *) __gluTessErrorString(errorCode - (GLU_TESS_ERROR1 - 1));
    }

    return (const GLubyte *) 0;
}

