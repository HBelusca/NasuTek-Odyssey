#pragma once

#define _KSDDK_

#define WIN32_NO_STATUS
#include <windows.h>
#include <ndk/iofuncs.h>
#include <ndk/obtypes.h>
#include <ndk/rtlfuncs.h>


#include <ks.h>

#define ROUND_DOWN(n, align) \
    (((ULONG)n) & ~((align) - 1l))

#define ROUND_UP(n, align) \
    ROUND_DOWN(((ULONG)n) + (align) - 1, (align))
