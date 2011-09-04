#ifndef PRECOMP_H__
#define PRECOMP_H__

#define BDAPLGIN_TRACE
#define BUILDING_KS
#define _KSDDK_
#include <dshow.h>
//#include <streams.h>
#include <ks.h>
#define __STREAMS__
#include <ksproxy.h>
#include <ksmedia.h>
#include <stdio.h>
#include <wchar.h>
#include <tchar.h>
#include <uuids.h>
#include <bdatypes.h>
#include <bdaiface.h>
#include <bdamedia.h>
#include <assert.h>

typedef HRESULT (CALLBACK *LPFNCREATEINSTANCE)(IUnknown* pUnkOuter, REFIID riid, LPVOID* ppvObject);

typedef struct
{
    const GUID* riid;
    LPFNCREATEINSTANCE lpfnCI;
} INTERFACE_TABLE;

/* classfactory.cpp */
IClassFactory *
CClassFactory_fnConstructor(
    LPFNCREATEINSTANCE lpfnCI,
    PLONG pcRefDll,
    IID * riidInst);

/* devicecontrol.cpp */
HRESULT
WINAPI
CBDADeviceControl_fnConstructor(
    IUnknown * pUnkOuter,
    REFIID riid,
    LPVOID * ppv);


/* pincontrol.cpp */
HRESULT
WINAPI
CBDAPinControl_fnConstructor(
    IUnknown * pUnkOuter,
    REFIID riid,
    LPVOID * ppv);

/* controlnode.cpp */

HRESULT
WINAPI
CControlNode_fnConstructor(
    IBaseFilter * pFilter,
    ULONG NodeType,
    ULONG PinId,
    REFIID riid,
    LPVOID * ppv);

/* frequencyfilter.cpp */

HRESULT
WINAPI
CBDAFrequencyFilter_fnConstructor(
    IKsPropertySet * pProperty,
    ULONG NodeId,
    REFIID riid,
    LPVOID * ppv);

/* signalstatistics.cpp */

HRESULT
WINAPI
CBDASignalStatistics_fnConstructor(
    IKsPropertySet * pProperty,
    ULONG NodeId,
    REFIID riid,
    LPVOID * ppv);

/* lnbinfo.cpp */

HRESULT
WINAPI
CBDALNBInfo_fnConstructor(
    IKsPropertySet * pProperty,
    ULONG NodeId,
    REFIID riid,
    LPVOID * ppv);

/* digitaldemo.cpp */
HRESULT
WINAPI
CBDADigitalDemodulator_fnConstructor(
    IKsPropertySet * pProperty,
    ULONG NodeId,
    REFIID riid,
    LPVOID * ppv);

extern const GUID IID_IKsObject;

#endif
