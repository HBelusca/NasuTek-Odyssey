#include "config.h"

extern "C" {

#include <ntddk.h>

};
#include "stddef.h"
#include "stdarg.h"

#include "inc/CrossNt.h"

#include "atapi.h"               // includes scsi.h
#include "ntdddisk.h"
#include "ntddscsi.h"
#include "bsmaster.h"
#include "uniata_ver.h"
#include "id_sata.h"

#ifndef UNIATA_CORE

#include "id_queue.h"

#ifdef ExAllocatePool
#undef ExAllocatePool
#endif
#define TAG_UNIATA 'ataU'
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,TAG_UNIATA)

#endif //UNIATA_CORE

#include "badblock.h"

