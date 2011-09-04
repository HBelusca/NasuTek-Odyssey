#define _KSDDK_

#include <stdio.h>

#include <ntifs.h>
#define NDEBUG
//#define YDEBUG
#include <debug.h>
#include <portcls.h>
#include <kcom.h>
#include <pseh/pseh2.h>

#include <ntimage.h>
#include <ndk/ldrfuncs.h>

#include "ksfunc.h"
#include "bdamedia.h"
#include <swenum.h>

#define TAG_DEVICE_HEADER 'KSDH'
#define REG_PINFLAG_B_MANY 0x4 /* strmif.h */
#define MERIT_DO_NOT_USE 0x200000 /* dshow.h */

#define DEFINE_KSPROPERTY_PINPROPOSEDATAFORMAT(PinSet,\
    PropGeneral, PropInstances, PropIntersection)\
DEFINE_KSPROPERTY_TABLE(PinSet) {\
    DEFINE_KSPROPERTY_ITEM_PIN_CINSTANCES(PropInstances),\
    DEFINE_KSPROPERTY_ITEM_PIN_CTYPES(PropGeneral),\
    DEFINE_KSPROPERTY_ITEM_PIN_DATAFLOW(PropGeneral),\
    DEFINE_KSPROPERTY_ITEM_PIN_DATARANGES(PropGeneral),\
    DEFINE_KSPROPERTY_ITEM_PIN_DATAINTERSECTION(PropIntersection),\
    DEFINE_KSPROPERTY_ITEM_PIN_INTERFACES(PropGeneral),\
    DEFINE_KSPROPERTY_ITEM_PIN_MEDIUMS(PropGeneral),\
    DEFINE_KSPROPERTY_ITEM_PIN_COMMUNICATION(PropGeneral),\
    DEFINE_KSPROPERTY_ITEM_PIN_CATEGORY(PropGeneral),\
    DEFINE_KSPROPERTY_ITEM_PIN_NAME(PropGeneral),\
    DEFINE_KSPROPERTY_ITEM_PIN_CONSTRAINEDDATARANGES(PropGeneral),\
    DEFINE_KSPROPERTY_ITEM_PIN_PROPOSEDATAFORMAT(PropGeneral)\
}

#define DEFINE_KSPROPERTY_CONNECTIONSET(PinSet,\
    PropStateHandler, PropDataFormatHandler, PropAllocatorFraming)\
DEFINE_KSPROPERTY_TABLE(PinSet) {\
    DEFINE_KSPROPERTY_ITEM_CONNECTION_STATE(PropStateHandler, PropStateHandler),\
    DEFINE_KSPROPERTY_ITEM_CONNECTION_DATAFORMAT(PropDataFormatHandler, PropDataFormatHandler),\
    DEFINE_KSPROPERTY_ITEM_CONNECTION_ALLOCATORFRAMING_EX(PropAllocatorFraming)\
}

#define DEFINE_KSPROPERTY_STREAMSET(PinSet,\
    PropStreamAllocator, PropMasterClock, PropPipeId)\
DEFINE_KSPROPERTY_TABLE(PinSet) {\
    DEFINE_KSPROPERTY_ITEM_STREAM_ALLOCATOR(PropStreamAllocator, PropStreamAllocator),\
    DEFINE_KSPROPERTY_ITEM_STREAM_MASTERCLOCK(PropMasterClock, PropMasterClock),\
    DEFINE_KSPROPERTY_ITEM_STREAM_PIPE_ID(PropPipeId, PropPipeId)\
}
