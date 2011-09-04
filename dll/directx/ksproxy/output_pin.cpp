/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey WDM Streaming ActiveMovie Proxy
 * FILE:            dll/directx/ksproxy/output_pin.cpp
 * PURPOSE:         OutputPin of Proxy Filter
 *
 * PROGRAMMERS:     Johannes Anderwald (janderwald@odyssey.org)
 */
#include "precomp.h"

class COutputPin : public IPin,
                   public IKsObject,
                   public IKsPropertySet,
                   public IStreamBuilder,
                   public IKsPinFactory,
                   public ISpecifyPropertyPages,
                   public IKsPinEx,
                   public IKsPinPipe,
                   public IKsControl,
                   public IKsAggregateControl,
                   public IQualityControl,
                   public IMediaSeeking,
                   public IAMBufferNegotiation,
                   public IAMStreamConfig,
                   public IMemAllocatorNotifyCallbackTemp

{
public:
    typedef std::vector<IUnknown *>ProxyPluginVector;

    STDMETHODIMP QueryInterface( REFIID InterfaceId, PVOID* Interface);

    STDMETHODIMP_(ULONG) AddRef()
    {
        InterlockedIncrement(&m_Ref);
        return m_Ref;
    }
    STDMETHODIMP_(ULONG) Release()
    {
        InterlockedDecrement(&m_Ref);
        if (!m_Ref)
        {
            //delete this;
            return 0;
        }
        return m_Ref;
    }

    //IKsPin
    HRESULT STDMETHODCALLTYPE KsQueryMediums(PKSMULTIPLE_ITEM* MediumList);
    HRESULT STDMETHODCALLTYPE KsQueryInterfaces(PKSMULTIPLE_ITEM* InterfaceList);
    HRESULT STDMETHODCALLTYPE KsCreateSinkPinHandle(KSPIN_INTERFACE& Interface, KSPIN_MEDIUM& Medium);
    HRESULT STDMETHODCALLTYPE KsGetCurrentCommunication(KSPIN_COMMUNICATION *Communication, KSPIN_INTERFACE *Interface, KSPIN_MEDIUM *Medium);
    HRESULT STDMETHODCALLTYPE KsPropagateAcquire();
    HRESULT STDMETHODCALLTYPE KsDeliver(IMediaSample* Sample, ULONG Flags);
    HRESULT STDMETHODCALLTYPE KsMediaSamplesCompleted(PKSSTREAM_SEGMENT StreamSegment);
    IMemAllocator * STDMETHODCALLTYPE KsPeekAllocator(KSPEEKOPERATION Operation);
    HRESULT STDMETHODCALLTYPE KsReceiveAllocator(IMemAllocator *MemAllocator);
    HRESULT STDMETHODCALLTYPE KsRenegotiateAllocator();
    LONG STDMETHODCALLTYPE KsIncrementPendingIoCount();
    LONG STDMETHODCALLTYPE KsDecrementPendingIoCount();
    HRESULT STDMETHODCALLTYPE KsQualityNotify(ULONG Proportion, REFERENCE_TIME TimeDelta);
    // IKsPinEx
    VOID STDMETHODCALLTYPE KsNotifyError(IMediaSample* Sample, HRESULT hr);

    //IKsPinPipe
    HRESULT STDMETHODCALLTYPE KsGetPinFramingCache(PKSALLOCATOR_FRAMING_EX *FramingEx, PFRAMING_PROP FramingProp, FRAMING_CACHE_OPS Option);
    HRESULT STDMETHODCALLTYPE KsSetPinFramingCache(PKSALLOCATOR_FRAMING_EX FramingEx, PFRAMING_PROP FramingProp, FRAMING_CACHE_OPS Option);
    IPin* STDMETHODCALLTYPE KsGetConnectedPin();
    IKsAllocatorEx* STDMETHODCALLTYPE KsGetPipe(KSPEEKOPERATION Operation);
    HRESULT STDMETHODCALLTYPE KsSetPipe(IKsAllocatorEx *KsAllocator);
    ULONG STDMETHODCALLTYPE KsGetPipeAllocatorFlag();
    HRESULT STDMETHODCALLTYPE KsSetPipeAllocatorFlag(ULONG Flag);
    GUID STDMETHODCALLTYPE KsGetPinBusCache();
    HRESULT STDMETHODCALLTYPE KsSetPinBusCache(GUID Bus);
    PWCHAR STDMETHODCALLTYPE KsGetPinName();
    PWCHAR STDMETHODCALLTYPE KsGetFilterName();

    //IPin methods
    HRESULT STDMETHODCALLTYPE Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
    HRESULT STDMETHODCALLTYPE ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt);
    HRESULT STDMETHODCALLTYPE Disconnect();
    HRESULT STDMETHODCALLTYPE ConnectedTo(IPin **pPin);
    HRESULT STDMETHODCALLTYPE ConnectionMediaType(AM_MEDIA_TYPE *pmt);
    HRESULT STDMETHODCALLTYPE QueryPinInfo(PIN_INFO *pInfo);
    HRESULT STDMETHODCALLTYPE QueryDirection(PIN_DIRECTION *pPinDir);
    HRESULT STDMETHODCALLTYPE QueryId(LPWSTR *Id);
    HRESULT STDMETHODCALLTYPE QueryAccept(const AM_MEDIA_TYPE *pmt);
    HRESULT STDMETHODCALLTYPE EnumMediaTypes(IEnumMediaTypes **ppEnum);
    HRESULT STDMETHODCALLTYPE QueryInternalConnections(IPin **apPin, ULONG *nPin);
    HRESULT STDMETHODCALLTYPE EndOfStream();
    HRESULT STDMETHODCALLTYPE BeginFlush();
    HRESULT STDMETHODCALLTYPE EndFlush();
    HRESULT STDMETHODCALLTYPE NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

    // ISpecifyPropertyPages
    HRESULT STDMETHODCALLTYPE GetPages(CAUUID *pPages);

    //IKsObject methods
    HANDLE STDMETHODCALLTYPE KsGetObjectHandle();

    //IKsPropertySet
    HRESULT STDMETHODCALLTYPE Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData);
    HRESULT STDMETHODCALLTYPE Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned);
    HRESULT STDMETHODCALLTYPE QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);

    //IKsControl
    HRESULT STDMETHODCALLTYPE KsProperty(PKSPROPERTY Property, ULONG PropertyLength, LPVOID PropertyData, ULONG DataLength, ULONG* BytesReturned);
    HRESULT STDMETHODCALLTYPE KsMethod(PKSMETHOD Method, ULONG MethodLength, LPVOID MethodData, ULONG DataLength, ULONG* BytesReturned);
    HRESULT STDMETHODCALLTYPE KsEvent(PKSEVENT Event, ULONG EventLength, LPVOID EventData, ULONG DataLength, ULONG* BytesReturned);

    //IStreamBuilder
    HRESULT STDMETHODCALLTYPE Render(IPin *ppinOut, IGraphBuilder *pGraph);
    HRESULT STDMETHODCALLTYPE Backout(IPin *ppinOut, IGraphBuilder *pGraph);

    //IKsPinFactory
    HRESULT STDMETHODCALLTYPE KsPinFactory(ULONG* PinFactory);

    //IKsAggregateControl
    HRESULT STDMETHODCALLTYPE KsAddAggregate(IN REFGUID AggregateClass);
    HRESULT STDMETHODCALLTYPE KsRemoveAggregate(REFGUID AggregateClass);

    //IQualityControl
    HRESULT STDMETHODCALLTYPE Notify(IBaseFilter *pSelf, Quality q);
    HRESULT STDMETHODCALLTYPE SetSink(IQualityControl *piqc);

    //IMediaSeeking
    HRESULT STDMETHODCALLTYPE GetCapabilities(DWORD *pCapabilities);
    HRESULT STDMETHODCALLTYPE CheckCapabilities(DWORD *pCapabilities);
    HRESULT STDMETHODCALLTYPE IsFormatSupported(const GUID *pFormat);
    HRESULT STDMETHODCALLTYPE QueryPreferredFormat(GUID *pFormat);
    HRESULT STDMETHODCALLTYPE GetTimeFormat(GUID *pFormat);
    HRESULT STDMETHODCALLTYPE IsUsingTimeFormat(const GUID *pFormat);
    HRESULT STDMETHODCALLTYPE SetTimeFormat(const GUID *pFormat);
    HRESULT STDMETHODCALLTYPE GetDuration(LONGLONG *pDuration);
    HRESULT STDMETHODCALLTYPE GetStopPosition(LONGLONG *pStop);
    HRESULT STDMETHODCALLTYPE GetCurrentPosition(LONGLONG *pCurrent);
    HRESULT STDMETHODCALLTYPE ConvertTimeFormat(LONGLONG *pTarget, const GUID *pTargetFormat, LONGLONG Source, const GUID *pSourceFormat);
    HRESULT STDMETHODCALLTYPE SetPositions(LONGLONG *pCurrent, DWORD dwCurrentFlags, LONGLONG *pStop, DWORD dwStopFlags);
    HRESULT STDMETHODCALLTYPE GetPositions(LONGLONG *pCurrent, LONGLONG *pStop);
    HRESULT STDMETHODCALLTYPE GetAvailable(LONGLONG *pEarliest, LONGLONG *pLatest);
    HRESULT STDMETHODCALLTYPE SetRate(double dRate);
    HRESULT STDMETHODCALLTYPE GetRate(double *pdRate);
    HRESULT STDMETHODCALLTYPE GetPreroll(LONGLONG *pllPreroll);

    //IAMBufferNegotiation
    HRESULT STDMETHODCALLTYPE SuggestAllocatorProperties(const ALLOCATOR_PROPERTIES *pprop);
    HRESULT STDMETHODCALLTYPE GetAllocatorProperties(ALLOCATOR_PROPERTIES *pprop);

    //IAMStreamConfig
    HRESULT STDMETHODCALLTYPE SetFormat(AM_MEDIA_TYPE *pmt);
    HRESULT STDMETHODCALLTYPE GetFormat(AM_MEDIA_TYPE **ppmt);
    HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities(int *piCount, int *piSize);
    HRESULT STDMETHODCALLTYPE GetStreamCaps(int iIndex, AM_MEDIA_TYPE **ppmt, BYTE *pSCC);

    //IMemAllocatorNotifyCallbackTemp
    HRESULT STDMETHODCALLTYPE NotifyRelease();

    //---------------------------------------------------------------
    COutputPin(IBaseFilter * ParentFilter, LPCWSTR PinName, ULONG PinId, KSPIN_COMMUNICATION Communication);
    virtual ~COutputPin();
    HRESULT STDMETHODCALLTYPE CheckFormat(const AM_MEDIA_TYPE *pmt);
    HRESULT STDMETHODCALLTYPE CreatePin(const AM_MEDIA_TYPE *pmt);
    HRESULT STDMETHODCALLTYPE CreatePinHandle(PKSPIN_MEDIUM Medium, PKSPIN_INTERFACE Interface, const AM_MEDIA_TYPE *pmt);
    HRESULT WINAPI IoProcessRoutine();
    HRESULT WINAPI InitializeIOThread();
    HRESULT STDMETHODCALLTYPE GetSupportedSets(LPGUID * pOutGuid, PULONG NumGuids);
    HRESULT STDMETHODCALLTYPE LoadProxyPlugins(LPGUID pGuids, ULONG NumGuids);

    friend DWORD WINAPI COutputPin_IoThreadStartup(LPVOID lpParameter);
    friend HRESULT STDMETHODCALLTYPE COutputPin_SetState(IPin * Pin, KSSTATE State);

protected:
    LONG m_Ref;
    IBaseFilter * m_ParentFilter;
    LPCWSTR m_PinName;
    HANDLE m_hPin;
    ULONG m_PinId;
    IPin * m_Pin;
    IKsAllocatorEx * m_KsAllocatorEx;
    ULONG m_PipeAllocatorFlag;
    BOOL m_bPinBusCacheInitialized;
    GUID m_PinBusCache;
    LPWSTR m_FilterName;
    FRAMING_PROP m_FramingProp[4];
    PKSALLOCATOR_FRAMING_EX m_FramingEx[4];

    IMemAllocator * m_MemAllocator;
    IMemInputPin * m_MemInputPin;
    LONG m_IoCount;
    KSPIN_COMMUNICATION m_Communication;
    KSPIN_INTERFACE m_Interface;
    KSPIN_MEDIUM m_Medium;
    AM_MEDIA_TYPE m_MediaFormat;
    ALLOCATOR_PROPERTIES m_Properties;
    IKsInterfaceHandler * m_InterfaceHandler;

    HANDLE m_hStartEvent;
    HANDLE m_hBufferAvailable;
    HANDLE m_hStopEvent;
    BOOL m_StopInProgress;
    BOOL m_IoThreadStarted;

    KSSTATE m_State;
    CRITICAL_SECTION m_Lock;

    ProxyPluginVector m_Plugins;
};

COutputPin::~COutputPin()
{
}

COutputPin::COutputPin(
    IBaseFilter * ParentFilter,
    LPCWSTR PinName,
    ULONG PinId,
    KSPIN_COMMUNICATION Communication) : m_Ref(0),
                                         m_ParentFilter(ParentFilter),
                                         m_PinName(PinName),
                                         m_hPin(INVALID_HANDLE_VALUE),
                                         m_PinId(PinId),
                                         m_Pin(0),
                                         m_KsAllocatorEx(0),
                                         m_PipeAllocatorFlag(0),
                                         m_bPinBusCacheInitialized(0),
                                         m_FilterName(0),
                                         m_MemAllocator(0),
                                         m_MemInputPin(0),
                                         m_IoCount(0),
                                         m_Communication(Communication),
                                         m_InterfaceHandler(0),
                                         m_hStartEvent(0),
                                         m_hBufferAvailable(0),
                                         m_hStopEvent(0),
                                         m_StopInProgress(0),
                                         m_IoThreadStarted(0),
                                         m_State(KSSTATE_STOP),
                                         m_Plugins()
{
    HRESULT hr;
    IKsObject * KsObjectParent;

    hr = m_ParentFilter->QueryInterface(IID_IKsObject, (LPVOID*)&KsObjectParent);
    assert(hr == S_OK);

    ZeroMemory(m_FramingProp, sizeof(m_FramingProp));
    ZeroMemory(m_FramingEx, sizeof(m_FramingEx));
    ZeroMemory(&m_MediaFormat, sizeof(AM_MEDIA_TYPE));

    hr = KsGetMediaType(0, &m_MediaFormat, KsObjectParent->KsGetObjectHandle(), m_PinId);

#ifdef KSPROXY_TRACE
    WCHAR Buffer[100];
    swprintf(Buffer, L"COutputPin::COutputPin Format %p pbFormat %lu\n", &m_MediaFormat, m_MediaFormat.cbFormat);
    OutputDebugStringW(Buffer);
#endif

    assert(hr == S_OK);

    InitializeCriticalSection(&m_Lock);

    KsObjectParent->Release();
};

HRESULT
STDMETHODCALLTYPE
COutputPin::QueryInterface(
    IN  REFIID refiid,
    OUT PVOID* Output)
{
    *Output = NULL;
    if (IsEqualGUID(refiid, IID_IUnknown) ||
        IsEqualGUID(refiid, IID_IPin))
    {
#ifdef KSPROXY_TRACE
        OutputDebugStringW(L"COutputPin::QueryInterface IID_IPin\n");
#endif
        *Output = PVOID(this);
        reinterpret_cast<IUnknown*>(*Output)->AddRef();
        return NOERROR;
    }
    else if (IsEqualGUID(refiid, IID_IKsObject))
    {
        if (m_hPin == INVALID_HANDLE_VALUE)
        {
            HRESULT hr = CreatePin(&m_MediaFormat);
            if (FAILED(hr))
                return hr;
        }
#ifdef KSPROXY_TRACE
        OutputDebugStringW(L"COutputPin::QueryInterface IID_IKsObject\n");
#endif
        *Output = (IKsObject*)(this);
        reinterpret_cast<IKsObject*>(*Output)->AddRef();
        return NOERROR;
    }
    else if (IsEqualGUID(refiid, IID_IKsPin) || IsEqualGUID(refiid, IID_IKsPinEx))
    {
        *Output = (IKsPinEx*)(this);
        reinterpret_cast<IKsPinEx*>(*Output)->AddRef();
        return NOERROR;
    }
    else if (IsEqualGUID(refiid, IID_IKsPinPipe))
    {
        *Output = (IKsPinPipe*)(this);
        reinterpret_cast<IKsPinPipe*>(*Output)->AddRef();
        return NOERROR;
    }
    else if (IsEqualGUID(refiid, IID_IKsAggregateControl))
    {
        *Output = (IKsAggregateControl*)(this);
        reinterpret_cast<IKsAggregateControl*>(*Output)->AddRef();
        return NOERROR;
    }
    else if (IsEqualGUID(refiid, IID_IQualityControl))
    {
        *Output = (IQualityControl*)(this);
        reinterpret_cast<IQualityControl*>(*Output)->AddRef();
        return NOERROR;
    }
    else if (IsEqualGUID(refiid, IID_IKsPropertySet))
    {
        if (m_hPin == INVALID_HANDLE_VALUE)
        {
            HRESULT hr = CreatePin(&m_MediaFormat);
            if (FAILED(hr))
                return hr;
        }
#ifdef KSPROXY_TRACE
        OutputDebugStringW(L"COutputPin::QueryInterface IID_IKsPropertySet\n");
#endif
        *Output = (IKsPropertySet*)(this);
        reinterpret_cast<IKsPropertySet*>(*Output)->AddRef();
        return NOERROR;
    }
    else if (IsEqualGUID(refiid, IID_IKsControl))
    {
#ifdef KSPROXY_TRACE
        OutputDebugStringW(L"COutputPin::QueryInterface IID_IKsControl\n");
#endif
        *Output = (IKsControl*)(this);
        reinterpret_cast<IKsControl*>(*Output)->AddRef();
        return NOERROR;
    }
#if 0
    else if (IsEqualGUID(refiid, IID_IStreamBuilder))
    {
        *Output = (IStreamBuilder*)(this);
        reinterpret_cast<IStreamBuilder*>(*Output)->AddRef();
        return NOERROR;
    }
#endif
    else if (IsEqualGUID(refiid, IID_IKsPinFactory))
    {
#ifdef KSPROXY_TRACE
        OutputDebugStringW(L"COutputPin::QueryInterface IID_IKsPinFactory\n");
#endif
        *Output = (IKsPinFactory*)(this);
        reinterpret_cast<IKsPinFactory*>(*Output)->AddRef();
        return NOERROR;
    }
    else if (IsEqualGUID(refiid, IID_ISpecifyPropertyPages))
    {
#ifdef KSPROXY_TRACE
        OutputDebugStringW(L"COutputPin::QueryInterface IID_ISpecifyPropertyPages\n");
#endif
        *Output = (ISpecifyPropertyPages*)(this);
        reinterpret_cast<ISpecifyPropertyPages*>(*Output)->AddRef();
        return NOERROR;
    }
    else if (IsEqualGUID(refiid, IID_IMediaSeeking))
    {
        *Output = (IMediaSeeking*)(this);
        reinterpret_cast<IMediaSeeking*>(*Output)->AddRef();
        return NOERROR;
    }
    else if (IsEqualGUID(refiid, IID_IAMBufferNegotiation))
    {
        *Output = (IAMBufferNegotiation*)(this);
        reinterpret_cast<IAMBufferNegotiation*>(*Output)->AddRef();
        return NOERROR;
    }
    else if (IsEqualGUID(refiid, IID_IAMStreamConfig))
    {
        *Output = (IAMStreamConfig*)(this);
        reinterpret_cast<IAMStreamConfig*>(*Output)->AddRef();
        return NOERROR;
    }
    else if (IsEqualGUID(refiid, IID_IMemAllocatorNotifyCallbackTemp))
    {
        *Output = (IMemAllocatorNotifyCallbackTemp*)(this);
        reinterpret_cast<IMemAllocatorNotifyCallbackTemp*>(*Output)->AddRef();
        return NOERROR;
    }

#ifdef KSPROXY_TRACE
    WCHAR Buffer[MAX_PATH];
    LPOLESTR lpstr;
    StringFromCLSID(refiid, &lpstr);
    swprintf(Buffer, L"COutputPin::QueryInterface: NoInterface for %s PinId %u PinName %s\n", lpstr, m_PinId, m_PinName);
    OutputDebugStringW(Buffer);
    CoTaskMemFree(lpstr);
#endif

    return E_NOINTERFACE;
}

//-------------------------------------------------------------------
// IAMBufferNegotiation interface
//
HRESULT
STDMETHODCALLTYPE
COutputPin::SuggestAllocatorProperties(
    const ALLOCATOR_PROPERTIES *pprop)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::SuggestAllocatorProperties\n");
#endif

    if (m_Pin)
    {
        // pin is already connected
        return VFW_E_ALREADY_CONNECTED;
    }

    CopyMemory(&m_Properties, pprop, sizeof(ALLOCATOR_PROPERTIES));
    return NOERROR;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::GetAllocatorProperties(
    ALLOCATOR_PROPERTIES *pprop)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::GetAllocatorProperties\n");
#endif

    if (!m_Pin)
    {
        // you should call this method AFTER you connected
        return E_UNEXPECTED;
    }

    if (!m_KsAllocatorEx)
    {
        // something went wrong while creating the allocator
        return E_FAIL;
    }

    CopyMemory(pprop, &m_Properties, sizeof(ALLOCATOR_PROPERTIES));
    return NOERROR;
}

//-------------------------------------------------------------------
// IAMStreamConfig interface
//
HRESULT
STDMETHODCALLTYPE
COutputPin::SetFormat(
    AM_MEDIA_TYPE *pmt)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::SetFormat NotImplemented\n");
#endif
    return E_NOTIMPL;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::GetFormat(AM_MEDIA_TYPE **ppmt)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::GetFormat NotImplemented\n");
#endif
    return E_NOTIMPL;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::GetNumberOfCapabilities(
    int *piCount,
    int *piSize)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::GetNumberOfCapabilities NotImplemented\n");
#endif
    return E_NOTIMPL;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::GetStreamCaps(
    int iIndex,
    AM_MEDIA_TYPE **ppmt,
    BYTE *pSCC)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::GetStreamCaps NotImplemented\n");
#endif
    return E_NOTIMPL;
}

//-------------------------------------------------------------------
// IMemAllocatorNotifyCallbackTemp interface
//
HRESULT
STDMETHODCALLTYPE
COutputPin::NotifyRelease()
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::NotifyRelease\n");
#endif

    // notify thread of new available sample
    SetEvent(m_hBufferAvailable);

    return NOERROR;
}

//-------------------------------------------------------------------
// IMediaSeeking interface
//
HRESULT
STDMETHODCALLTYPE
COutputPin::GetCapabilities(
    DWORD *pCapabilities)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->GetCapabilities(pCapabilities);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::CheckCapabilities(
    DWORD *pCapabilities)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->CheckCapabilities(pCapabilities);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::IsFormatSupported(
    const GUID *pFormat)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->IsFormatSupported(pFormat);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::QueryPreferredFormat(
    GUID *pFormat)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->QueryPreferredFormat(pFormat);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::GetTimeFormat(
    GUID *pFormat)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->GetTimeFormat(pFormat);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::IsUsingTimeFormat(
    const GUID *pFormat)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->IsUsingTimeFormat(pFormat);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::SetTimeFormat(
    const GUID *pFormat)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->SetTimeFormat(pFormat);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::GetDuration(
    LONGLONG *pDuration)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->GetDuration(pDuration);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::GetStopPosition(
    LONGLONG *pStop)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->GetStopPosition(pStop);

    FilterMediaSeeking->Release();
    return hr;
}


HRESULT
STDMETHODCALLTYPE
COutputPin::GetCurrentPosition(
    LONGLONG *pCurrent)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->GetCurrentPosition(pCurrent);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::ConvertTimeFormat(
    LONGLONG *pTarget,
    const GUID *pTargetFormat,
    LONGLONG Source,
    const GUID *pSourceFormat)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->ConvertTimeFormat(pTarget, pTargetFormat, Source, pSourceFormat);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::SetPositions(
    LONGLONG *pCurrent,
    DWORD dwCurrentFlags,
    LONGLONG *pStop,
    DWORD dwStopFlags)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->SetPositions(pCurrent, dwCurrentFlags, pStop, dwStopFlags);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::GetPositions(
    LONGLONG *pCurrent,
    LONGLONG *pStop)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->GetPositions(pCurrent, pStop);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::GetAvailable(
    LONGLONG *pEarliest,
    LONGLONG *pLatest)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->GetAvailable(pEarliest, pLatest);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::SetRate(
    double dRate)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->SetRate(dRate);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::GetRate(
    double *pdRate)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->GetRate(pdRate);

    FilterMediaSeeking->Release();
    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::GetPreroll(
    LONGLONG *pllPreroll)
{
    IMediaSeeking * FilterMediaSeeking;
    HRESULT hr;

    hr = m_ParentFilter->QueryInterface(IID_IMediaSeeking, (LPVOID*)&FilterMediaSeeking);
    if (FAILED(hr))
        return hr;

    hr = FilterMediaSeeking->GetPreroll(pllPreroll);

    FilterMediaSeeking->Release();
    return hr;
}

//-------------------------------------------------------------------
// IQualityControl interface
//
HRESULT
STDMETHODCALLTYPE
COutputPin::Notify(
    IBaseFilter *pSelf,
    Quality q)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::Notify NotImplemented\n");
#endif
    return E_NOTIMPL;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::SetSink(
    IQualityControl *piqc)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::SetSink NotImplemented\n");
#endif
    return E_NOTIMPL;
}


//-------------------------------------------------------------------
// IKsAggregateControl interface
//
HRESULT
STDMETHODCALLTYPE
COutputPin::KsAddAggregate(
    IN REFGUID AggregateClass)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::KsAddAggregate NotImplemented\n");
#endif
    return E_NOTIMPL;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsRemoveAggregate(
    REFGUID AggregateClass)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::KsRemoveAggregate NotImplemented\n");
#endif
    return E_NOTIMPL;
}


//-------------------------------------------------------------------
// IKsPin
//

HRESULT
STDMETHODCALLTYPE
COutputPin::KsQueryMediums(
    PKSMULTIPLE_ITEM* MediumList)
{
    HRESULT hr;
    HANDLE hFilter;
    IKsObject * KsObjectParent;

    hr = m_ParentFilter->QueryInterface(IID_IKsObject, (LPVOID*)&KsObjectParent);
    if (FAILED(hr))
        return E_NOINTERFACE;

    hFilter = KsObjectParent->KsGetObjectHandle();

    if (hFilter)
        hr = KsGetMultiplePinFactoryItems(hFilter, m_PinId, KSPROPERTY_PIN_MEDIUMS, (PVOID*)MediumList);
    else
        hr = E_HANDLE;

    KsObjectParent->Release();

    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsQueryInterfaces(
    PKSMULTIPLE_ITEM* InterfaceList)
{
    HRESULT hr;
    HANDLE hFilter;
    IKsObject * KsObjectParent;

    hr = m_ParentFilter->QueryInterface(IID_IKsObject, (LPVOID*)&KsObjectParent);
    if (FAILED(hr))
        return hr;

    hFilter = KsObjectParent->KsGetObjectHandle();

    if (hFilter)
        hr = KsGetMultiplePinFactoryItems(hFilter, m_PinId, KSPROPERTY_PIN_INTERFACES, (PVOID*)InterfaceList);
    else
        hr = E_HANDLE;

    KsObjectParent->Release();

    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsCreateSinkPinHandle(
    KSPIN_INTERFACE& Interface,
    KSPIN_MEDIUM& Medium)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::KsCreateSinkPinHandle NotImplemented\n");
#endif
    return E_NOTIMPL;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsGetCurrentCommunication(
    KSPIN_COMMUNICATION *Communication,
    KSPIN_INTERFACE *Interface,
    KSPIN_MEDIUM *Medium)
{
    if (Communication)
    {
        *Communication = m_Communication;
    }

    if (Interface)
    {
        if (!m_hPin)
            return VFW_E_NOT_CONNECTED;

        CopyMemory(Interface, &m_Interface, sizeof(KSPIN_INTERFACE));
    }

    if (Medium)
    {
        if (!m_hPin)
            return VFW_E_NOT_CONNECTED;

        CopyMemory(Medium, &m_Medium, sizeof(KSPIN_MEDIUM));
    }
    return NOERROR;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsPropagateAcquire()
{
    KSPROPERTY Property;
    KSSTATE State;
    ULONG BytesReturned;
    HRESULT hr;

#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::KsPropagateAcquire\n");
#endif

    assert(m_hPin != INVALID_HANDLE_VALUE);

    Property.Set = KSPROPSETID_Connection;
    Property.Id = KSPROPERTY_CONNECTION_STATE;
    Property.Flags = KSPROPERTY_TYPE_SET;

    State = KSSTATE_ACQUIRE;

    hr = KsProperty(&Property, sizeof(KSPROPERTY), (LPVOID)&State, sizeof(KSSTATE), &BytesReturned);
    if (SUCCEEDED(hr))
    {
        m_State = State;
    }

    //TODO
    //propagate to connected pin on the pipe

    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsDeliver(
    IMediaSample* Sample,
    ULONG Flags)
{
    return E_FAIL;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsMediaSamplesCompleted(PKSSTREAM_SEGMENT StreamSegment)
{
    return NOERROR;
}

IMemAllocator *
STDMETHODCALLTYPE
COutputPin::KsPeekAllocator(KSPEEKOPERATION Operation)
{
    if (Operation == KsPeekOperation_AddRef)
    {
        // add reference on allocator
        m_MemAllocator->AddRef();
    }

    return m_MemAllocator;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsReceiveAllocator(IMemAllocator *MemAllocator)
{
    if (MemAllocator)
    {
        MemAllocator->AddRef();
    }

    if (m_MemAllocator)
    {
        m_MemAllocator->Release();
    }

    m_MemAllocator = MemAllocator;
    return NOERROR;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsRenegotiateAllocator()
{
    return E_FAIL;
}

LONG
STDMETHODCALLTYPE
COutputPin::KsIncrementPendingIoCount()
{
    return InterlockedIncrement((volatile LONG*)&m_IoCount);
}

LONG
STDMETHODCALLTYPE
COutputPin::KsDecrementPendingIoCount()
{
    return InterlockedDecrement((volatile LONG*)&m_IoCount);
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsQualityNotify(
    ULONG Proportion,
    REFERENCE_TIME TimeDelta)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::KsQualityNotify NotImplemented\n");
#endif
    return E_NOTIMPL;
}

//-------------------------------------------------------------------
// IKsPinEx
//

VOID
STDMETHODCALLTYPE
COutputPin::KsNotifyError(
    IMediaSample* Sample,
    HRESULT hr)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::KsNotifyError NotImplemented\n");
#endif
}


//-------------------------------------------------------------------
// IKsPinPipe
//

HRESULT
STDMETHODCALLTYPE
COutputPin::KsGetPinFramingCache(
    PKSALLOCATOR_FRAMING_EX *FramingEx,
    PFRAMING_PROP FramingProp,
    FRAMING_CACHE_OPS Option)
{
    if (Option > Framing_Cache_Write || Option < Framing_Cache_ReadLast)
    {
        // invalid argument
        return E_INVALIDARG;
    }

    // get framing properties
    *FramingProp = m_FramingProp[Option];
    *FramingEx = m_FramingEx[Option];

    return NOERROR;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsSetPinFramingCache(
    PKSALLOCATOR_FRAMING_EX FramingEx,
    PFRAMING_PROP FramingProp,
    FRAMING_CACHE_OPS Option)
{
    ULONG Index;
    ULONG RefCount = 0;

    if (m_FramingEx[Option])
    {
        for(Index = 1; Index < 4; Index++)
        {
            if (m_FramingEx[Index] == m_FramingEx[Option])
                RefCount++;
        }

        if (RefCount == 1)
        {
            // existing framing is only used once
            CoTaskMemFree(m_FramingEx[Option]);
        }
    }

    // store framing
    m_FramingEx[Option] = FramingEx;
    m_FramingProp[Option] = *FramingProp;

    return S_OK;
}

IPin*
STDMETHODCALLTYPE
COutputPin::KsGetConnectedPin()
{
    return m_Pin;
}

IKsAllocatorEx*
STDMETHODCALLTYPE
COutputPin::KsGetPipe(
    KSPEEKOPERATION Operation)
{
    if (Operation == KsPeekOperation_AddRef)
    {
        if (m_KsAllocatorEx)
            m_KsAllocatorEx->AddRef();
    }
    return m_KsAllocatorEx;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsSetPipe(
    IKsAllocatorEx *KsAllocator)
{
    if (KsAllocator)
        KsAllocator->AddRef();

    if (m_KsAllocatorEx)
        m_KsAllocatorEx->Release();

    m_KsAllocatorEx = KsAllocator;
    return NOERROR;
}

ULONG
STDMETHODCALLTYPE
COutputPin::KsGetPipeAllocatorFlag()
{
    return m_PipeAllocatorFlag;
}


HRESULT
STDMETHODCALLTYPE
COutputPin::KsSetPipeAllocatorFlag(
    ULONG Flag)
{
    m_PipeAllocatorFlag = Flag;
    return NOERROR;
}

GUID
STDMETHODCALLTYPE
COutputPin::KsGetPinBusCache()
{
    if (!m_bPinBusCacheInitialized)
    {
        CopyMemory(&m_PinBusCache, &m_Medium.Set, sizeof(GUID));
        m_bPinBusCacheInitialized = TRUE;
    }

    return m_PinBusCache;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsSetPinBusCache(
    GUID Bus)
{
    CopyMemory(&m_PinBusCache, &Bus, sizeof(GUID));
    return NOERROR;
}

PWCHAR
STDMETHODCALLTYPE
COutputPin::KsGetPinName()
{
    return (PWCHAR)m_PinName;
}


PWCHAR
STDMETHODCALLTYPE
COutputPin::KsGetFilterName()
{
    return m_FilterName;
}

//-------------------------------------------------------------------
// ISpecifyPropertyPages
//

HRESULT
STDMETHODCALLTYPE
COutputPin::GetPages(CAUUID *pPages)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::GetPages NotImplemented\n");
#endif

    if (!pPages)
        return E_POINTER;

    pPages->cElems = 0;
    pPages->pElems = NULL;

    return S_OK;
}

//-------------------------------------------------------------------
// IKsPinFactory
//

HRESULT
STDMETHODCALLTYPE
COutputPin::KsPinFactory(
    ULONG* PinFactory)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::KsPinFactory\n");
#endif

    *PinFactory = m_PinId;
    return S_OK;
}


//-------------------------------------------------------------------
// IStreamBuilder
//

HRESULT
STDMETHODCALLTYPE
COutputPin::Render(
    IPin *ppinOut,
    IGraphBuilder *pGraph)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::Render\n");
#endif
    return S_OK;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::Backout(
    IPin *ppinOut,
    IGraphBuilder *pGraph)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::Backout\n");
#endif

    return S_OK;
}
//-------------------------------------------------------------------
// IKsObject
//
HANDLE
STDMETHODCALLTYPE
COutputPin::KsGetObjectHandle()
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::KsGetObjectHandle\n");
#endif

    assert(m_hPin != INVALID_HANDLE_VALUE);
    return m_hPin;
}

//-------------------------------------------------------------------
// IKsControl
//
HRESULT
STDMETHODCALLTYPE
COutputPin::KsProperty(
    PKSPROPERTY Property,
    ULONG PropertyLength,
    LPVOID PropertyData,
    ULONG DataLength,
    ULONG* BytesReturned)
{
    HRESULT hr;

    assert(m_hPin != INVALID_HANDLE_VALUE);

    hr = KsSynchronousDeviceControl(m_hPin, IOCTL_KS_PROPERTY, (PVOID)Property, PropertyLength, (PVOID)PropertyData, DataLength, BytesReturned);
#ifdef KSPROXY_TRACE
    WCHAR Buffer[100];
    LPOLESTR pstr;
    StringFromCLSID(Property->Set, &pstr);
    swprintf(Buffer, L"COutputPin::KsProperty Set %s Id %lu Flags %x hr %x\n", pstr, Property->Id, Property->Flags, hr);
    OutputDebugStringW(Buffer);
#endif

    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsMethod(
    PKSMETHOD Method,
    ULONG MethodLength,
    LPVOID MethodData,
    ULONG DataLength,
    ULONG* BytesReturned)
{
    assert(m_hPin != INVALID_HANDLE_VALUE);
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::KsMethod\n");
#endif
    return KsSynchronousDeviceControl(m_hPin, IOCTL_KS_METHOD, (PVOID)Method, MethodLength, (PVOID)MethodData, DataLength, BytesReturned);
}

HRESULT
STDMETHODCALLTYPE
COutputPin::KsEvent(
    PKSEVENT Event,
    ULONG EventLength,
    LPVOID EventData,
    ULONG DataLength,
    ULONG* BytesReturned)
{
    assert(m_hPin != INVALID_HANDLE_VALUE);

#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::KsEvent\n");
#endif

    if (EventLength)
        return KsSynchronousDeviceControl(m_hPin, IOCTL_KS_ENABLE_EVENT, (PVOID)Event, EventLength, (PVOID)EventData, DataLength, BytesReturned);
    else
        return KsSynchronousDeviceControl(m_hPin, IOCTL_KS_DISABLE_EVENT, (PVOID)Event, EventLength, NULL, 0, BytesReturned);
}


//-------------------------------------------------------------------
// IKsPropertySet
//
HRESULT
STDMETHODCALLTYPE
COutputPin::Set(
    REFGUID guidPropSet,
    DWORD dwPropID,
    LPVOID pInstanceData,
    DWORD cbInstanceData,
    LPVOID pPropData,
    DWORD cbPropData)
{
    ULONG BytesReturned;

    if (cbInstanceData)
    {
        PKSPROPERTY Property = (PKSPROPERTY)CoTaskMemAlloc(sizeof(KSPROPERTY) + cbInstanceData);
        if (!Property)
            return E_OUTOFMEMORY;

        Property->Set = guidPropSet;
        Property->Id = dwPropID;
        Property->Flags = KSPROPERTY_TYPE_SET;

        CopyMemory((Property+1), pInstanceData, cbInstanceData);

        HRESULT hr = KsProperty(Property, sizeof(KSPROPERTY) + cbInstanceData, pPropData, cbPropData, &BytesReturned);
        CoTaskMemFree(Property);
        return hr;
    }
    else
    {
        KSPROPERTY Property;

        Property.Set = guidPropSet;
        Property.Id = dwPropID;
        Property.Flags = KSPROPERTY_TYPE_SET;

        HRESULT hr = KsProperty(&Property, sizeof(KSPROPERTY), pPropData, cbPropData, &BytesReturned);
        return hr;
    }
}

HRESULT
STDMETHODCALLTYPE
COutputPin::Get(
    REFGUID guidPropSet,
    DWORD dwPropID,
    LPVOID pInstanceData,
    DWORD cbInstanceData,
    LPVOID pPropData,
    DWORD cbPropData,
    DWORD *pcbReturned)
{
    ULONG BytesReturned;

    if (cbInstanceData)
    {
        PKSPROPERTY Property = (PKSPROPERTY)CoTaskMemAlloc(sizeof(KSPROPERTY) + cbInstanceData);
        if (!Property)
            return E_OUTOFMEMORY;

        Property->Set = guidPropSet;
        Property->Id = dwPropID;
        Property->Flags = KSPROPERTY_TYPE_GET;

        CopyMemory((Property+1), pInstanceData, cbInstanceData);

        HRESULT hr = KsProperty(Property, sizeof(KSPROPERTY) + cbInstanceData, pPropData, cbPropData, &BytesReturned);
        CoTaskMemFree(Property);
        return hr;
    }
    else
    {
        KSPROPERTY Property;

        Property.Set = guidPropSet;
        Property.Id = dwPropID;
        Property.Flags = KSPROPERTY_TYPE_GET;

        HRESULT hr = KsProperty(&Property, sizeof(KSPROPERTY), pPropData, cbPropData, &BytesReturned);
        return hr;
    }
}

HRESULT
STDMETHODCALLTYPE
COutputPin::QuerySupported(
    REFGUID guidPropSet,
    DWORD dwPropID,
    DWORD *pTypeSupport)
{
    KSPROPERTY Property;
    ULONG BytesReturned;

#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::QuerySupported\n");
#endif

    Property.Set = guidPropSet;
    Property.Id = dwPropID;
    Property.Flags = KSPROPERTY_TYPE_SETSUPPORT;

    return KsProperty(&Property, sizeof(KSPROPERTY), pTypeSupport, sizeof(DWORD), &BytesReturned);
}


//-------------------------------------------------------------------
// IPin interface
//
HRESULT
STDMETHODCALLTYPE
COutputPin::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
    HRESULT hr;
    ALLOCATOR_PROPERTIES Properties;
    IMemAllocatorCallbackTemp *pMemCallback;
    LPGUID pGuid;
    ULONG NumGuids = 0;

#ifdef KSPROXY_TRACE
    WCHAR Buffer[200];
    OutputDebugStringW(L"COutputPin::Connect called\n");
#endif

    if (pmt)
    {
        hr = pReceivePin->QueryAccept(pmt);
        if (FAILED(hr))
            return hr;
    }
    else
    {
        // query accept
        hr = pReceivePin->QueryAccept(&m_MediaFormat);
        if (FAILED(hr))
            return hr;

         pmt = &m_MediaFormat;
    }

    if (m_hPin == INVALID_HANDLE_VALUE)
    {
        hr = CreatePin(pmt);
        if (FAILED(hr))
        {
#ifdef KSPROXY_TRACE
            swprintf(Buffer, L"COutputPin::Connect CreatePin handle failed with %lx\n", hr);
            OutputDebugStringW(Buffer);
#endif
            return hr;
        }
    }


    // query for IMemInput interface
    hr = pReceivePin->QueryInterface(IID_IMemInputPin, (void**)&m_MemInputPin);
    if (FAILED(hr))
    {
#ifdef KSPROXY_TRACE
        OutputDebugStringW(L"COutputPin::Connect no IMemInputPin interface\n");
#endif

        return hr;
    }

    // get input pin allocator properties
    ZeroMemory(&Properties, sizeof(ALLOCATOR_PROPERTIES));
    m_MemInputPin->GetAllocatorRequirements(&Properties);

    //FIXME determine allocator properties
    Properties.cBuffers = 32;
    Properties.cbBuffer = 2048 * 188; //2048 frames * MPEG2 TS Payload size
    Properties.cbAlign = 4;

    // get input pin allocator
#if 0
    hr = m_MemInputPin->GetAllocator(&m_MemAllocator);
    if (SUCCEEDED(hr))
    {
        // set allocator properties
        hr = m_MemAllocator->SetProperties(&Properties, &m_Properties);
        if (FAILED(hr))
            m_MemAllocator->Release();
    }
#endif

    if (1)
    {
        hr = CKsAllocator_Constructor(NULL, IID_IMemAllocator, (void**)&m_MemAllocator);
        if (FAILED(hr))
            return hr;

        // set allocator properties
        hr = m_MemAllocator->SetProperties(&Properties, &m_Properties);
        if (FAILED(hr))
        {
#ifdef KSPROXY_TRACE
            swprintf(Buffer, L"COutputPin::Connect IMemAllocator::SetProperties failed with hr %lx\n", hr);
            OutputDebugStringW(Buffer);
#endif
            m_MemAllocator->Release();
            m_MemInputPin->Release();
            return hr;
        }
    }

    // commit property changes
    hr = m_MemAllocator->Commit();
    if (FAILED(hr))
    {
#ifdef KSPROXY_TRACE
        swprintf(Buffer, L"COutputPin::Connect IMemAllocator::Commit failed with hr %lx\n", hr);
        OutputDebugStringW(Buffer);
#endif
        m_MemAllocator->Release();
        m_MemInputPin->Release();
        return hr;
    }

    // get callback interface
    hr = m_MemAllocator->QueryInterface(IID_IMemAllocatorCallbackTemp, (void**)&pMemCallback);
    if (FAILED(hr))
    {
#ifdef KSPROXY_TRACE
        swprintf(Buffer, L"COutputPin::Connect No IMemAllocatorCallbackTemp interface hr %lx\n", hr);
        OutputDebugStringW(Buffer);
#endif
        m_MemAllocator->Release();
        m_MemInputPin->Release();
        return hr;
    }

    // set notification routine
    hr = pMemCallback->SetNotify((IMemAllocatorNotifyCallbackTemp*)this);

    // release IMemAllocatorNotifyCallbackTemp interface
    pMemCallback->Release();

    if (FAILED(hr))
    {
#ifdef KSPROXY_TRACE
        swprintf(Buffer, L"COutputPin::Connect IMemAllocatorNotifyCallbackTemp::SetNotify failed hr %lx\n", hr);
        OutputDebugStringW(Buffer);
#endif
        m_MemAllocator->Release();
        m_MemInputPin->Release();
        return hr;
    }

    // now set allocator
    hr = m_MemInputPin->NotifyAllocator(m_MemAllocator, TRUE);
    if (FAILED(hr))
    {
#ifdef KSPROXY_TRACE
        swprintf(Buffer, L"COutputPin::Connect IMemInputPin::NotifyAllocator failed with hr %lx\n", hr);
        OutputDebugStringW(Buffer);
#endif
        m_MemAllocator->Release();
        m_MemInputPin->Release();
        return hr;
    }


    assert(m_hPin != INVALID_HANDLE_VALUE);

    // get all supported sets
    if (m_Plugins.size() == 0)
    {
        if (GetSupportedSets(&pGuid, &NumGuids))
        {
            // load all proxy plugins
            if (FAILED(LoadProxyPlugins(pGuid, NumGuids)));
            {
#ifdef KSPROXY_TRACE
                OutputDebugStringW(L"COutputPin::Connect LoadProxyPlugins failed\n");
#endif
            }
            // free sets
            CoTaskMemFree(pGuid);
        }
    }

    // receive connection;
    hr = pReceivePin->ReceiveConnection((IPin*)this, pmt);
    if (SUCCEEDED(hr))
    {
        // increment reference count
        pReceivePin->AddRef();
        m_Pin = pReceivePin;
#ifdef KSPROXY_TRACE
        OutputDebugStringW(L"COutputPin::Connect success\n");
#endif
    }
    else
    {
        m_MemInputPin->Release();
        m_MemAllocator->Release();
    }

    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt)
{
    return E_UNEXPECTED;
}
HRESULT
STDMETHODCALLTYPE
COutputPin::Disconnect( void)
{
#ifdef KSPROXY_TRACE
   OutputDebugStringW(L"COutputPin::Disconnect\n");
#endif

    if (!m_Pin)
    {
        // pin was not connected
        return S_FALSE;
    }

    //FIXME
    //check if filter is active

    m_Pin->Release();
    m_Pin = NULL;
    m_MemInputPin->Release();
    m_MemAllocator->Release();

    CloseHandle(m_hPin);
    m_hPin = INVALID_HANDLE_VALUE;

#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::Disconnect\n");
#endif
    return S_OK;
}
HRESULT
STDMETHODCALLTYPE
COutputPin::ConnectedTo(IPin **pPin)
{
#ifdef KSPROXY_TRACE
   OutputDebugStringW(L"COutputPin::ConnectedTo\n");
#endif

    if (!pPin)
        return E_POINTER;

    if (m_Pin)
    {
        // increment reference count
        m_Pin->AddRef();
        *pPin = m_Pin;
        return S_OK;
    }

    *pPin = NULL;
    return VFW_E_NOT_CONNECTED;
}
HRESULT
STDMETHODCALLTYPE
COutputPin::ConnectionMediaType(AM_MEDIA_TYPE *pmt)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::ConnectionMediaType called\n");
#endif

    return E_NOTIMPL;
}
HRESULT
STDMETHODCALLTYPE
COutputPin::QueryPinInfo(PIN_INFO *pInfo)
{
    wcscpy(pInfo->achName, m_PinName);
    pInfo->dir = PINDIR_OUTPUT;
    pInfo->pFilter = m_ParentFilter;
    m_ParentFilter->AddRef();

    return S_OK;
}
HRESULT
STDMETHODCALLTYPE
COutputPin::QueryDirection(PIN_DIRECTION *pPinDir)
{
    if (pPinDir)
    {
        *pPinDir = PINDIR_OUTPUT;
        return S_OK;
    }

    return E_POINTER;
}
HRESULT
STDMETHODCALLTYPE
COutputPin::QueryId(LPWSTR *Id)
{
    *Id = (LPWSTR)CoTaskMemAlloc((wcslen(m_PinName)+1)*sizeof(WCHAR));
    if (!*Id)
        return E_OUTOFMEMORY;

    wcscpy(*Id, m_PinName);
    return S_OK;
}
HRESULT
STDMETHODCALLTYPE
COutputPin::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
#ifdef KSPROXY_TRACE
    OutputDebugStringW(L"COutputPin::QueryAccept called\n");
#endif

    return E_NOTIMPL;
}
HRESULT
STDMETHODCALLTYPE
COutputPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    HRESULT hr;
    ULONG MediaTypeCount = 0, Index;
    AM_MEDIA_TYPE * MediaTypes;
    HANDLE hFilter;
    IKsObject * KsObjectParent;

    hr = m_ParentFilter->QueryInterface(IID_IKsObject, (LPVOID*)&KsObjectParent);
    if (FAILED(hr))
        return hr;

    // get parent filter handle
    hFilter = KsObjectParent->KsGetObjectHandle();

    // release IKsObject
    KsObjectParent->Release();

    // query media type count
    hr = KsGetMediaTypeCount(hFilter, m_PinId, &MediaTypeCount);
    if (FAILED(hr) || !MediaTypeCount)
    {
        return hr;
    }

    // allocate media types
    MediaTypes = (AM_MEDIA_TYPE*)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE) * MediaTypeCount);
    if (!MediaTypes)
    {
        // not enough memory
        return E_OUTOFMEMORY;
    }

    // zero media types
    ZeroMemory(MediaTypes, sizeof(AM_MEDIA_TYPE) * MediaTypeCount);

    for(Index = 0; Index < MediaTypeCount; Index++)
    {
        // get media type
        hr = KsGetMediaType(Index, &MediaTypes[Index], hFilter, m_PinId);
        if (FAILED(hr))
        {
            // failed
            CoTaskMemFree(MediaTypes);
            return hr;
        }
    }

    return CEnumMediaTypes_fnConstructor(MediaTypeCount, MediaTypes, IID_IEnumMediaTypes, (void**)ppEnum);
}
HRESULT
STDMETHODCALLTYPE
COutputPin::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
    return E_NOTIMPL;
}
HRESULT
STDMETHODCALLTYPE
COutputPin::EndOfStream( void)
{
    /* should be called only on input pins */
    return E_UNEXPECTED;
}
HRESULT
STDMETHODCALLTYPE
COutputPin::BeginFlush( void)
{
    /* should be called only on input pins */
    return E_UNEXPECTED;
}
HRESULT
STDMETHODCALLTYPE
COutputPin::EndFlush( void)
{
    /* should be called only on input pins */
    return E_UNEXPECTED;
}
HRESULT
STDMETHODCALLTYPE
COutputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    if (!m_Pin)
    {
        // we are not connected
        return VFW_E_NOT_CONNECTED;
    }

    return m_Pin->NewSegment(tStart, tStop, dRate);
}

//-------------------------------------------------------------------
HRESULT
STDMETHODCALLTYPE
COutputPin::CheckFormat(
    const AM_MEDIA_TYPE *pmt)
{
    PKSMULTIPLE_ITEM MultipleItem;
    PKSDATAFORMAT DataFormat;
    HRESULT hr;
    IKsObject * KsObjectParent;
    HANDLE hFilter;

    if (!pmt)
        return E_POINTER;

    // get IKsObject interface
    hr = m_ParentFilter->QueryInterface(IID_IKsObject, (LPVOID*)&KsObjectParent);
    if (FAILED(hr))
        return hr;

    // get parent filter handle
    hFilter = KsObjectParent->KsGetObjectHandle();

    // release IKsObject
    KsObjectParent->Release();

    if (!hFilter)
        return E_HANDLE;


    hr = KsGetMultiplePinFactoryItems(hFilter, m_PinId, KSPROPERTY_PIN_DATARANGES, (PVOID*)&MultipleItem);
    if (FAILED(hr))
        return S_FALSE;

    DataFormat = (PKSDATAFORMAT)(MultipleItem + 1);
    for(ULONG Index = 0; Index < MultipleItem->Count; Index++)
    {
        if (IsEqualGUID(pmt->majortype, DataFormat->MajorFormat) &&
            IsEqualGUID(pmt->subtype, DataFormat->SubFormat) &&
            IsEqualGUID(pmt->formattype, DataFormat->Specifier))
        {
            // format is supported
            CoTaskMemFree(MultipleItem);
            return S_OK;
        }
        DataFormat = (PKSDATAFORMAT)((ULONG_PTR)DataFormat + DataFormat->FormatSize);
    }
    //format is not supported
    CoTaskMemFree(MultipleItem);
    return S_FALSE;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::CreatePin(
    const AM_MEDIA_TYPE *pmt)
{
    PKSMULTIPLE_ITEM MediumList;
    PKSMULTIPLE_ITEM InterfaceList;
    PKSPIN_MEDIUM Medium;
    PKSPIN_INTERFACE Interface;
    IKsInterfaceHandler * InterfaceHandler;
    HRESULT hr;

    // query for pin medium
    hr = KsQueryMediums(&MediumList);
    if (FAILED(hr))
    {
#ifdef KSPROXY_TRACE
        WCHAR Buffer[100];
        swprintf(Buffer, L"COutputPin::CreatePin KsQueryMediums failed %lx\n", hr);
        OutputDebugStringW(Buffer);
#endif
        return hr;
    }

    // query for pin interface
    hr = KsQueryInterfaces(&InterfaceList);
    if (FAILED(hr))
    {
        // failed
#ifdef KSPROXY_TRACE
        WCHAR Buffer[100];
        swprintf(Buffer, L"COutputPin::CreatePin KsQueryInterfaces failed %lx\n", hr);
        OutputDebugStringW(Buffer);
#endif

        CoTaskMemFree(MediumList);
        return hr;
    }

    if (MediumList->Count)
    {
        //use first available medium
        Medium = (PKSPIN_MEDIUM)(MediumList + 1);
    }
    else
    {
        // default to standard medium
        Medium = &StandardPinMedium;
    }

    if (InterfaceList->Count)
    {
        //use first available interface
        Interface = (PKSPIN_INTERFACE)(InterfaceList + 1);
    }
    else
    {
        // default to standard interface
        Interface = &StandardPinInterface;
    }

    if (m_Communication != KSPIN_COMMUNICATION_BRIDGE && m_Communication != KSPIN_COMMUNICATION_NONE)
    {
        // now create pin
        hr = CreatePinHandle(Medium, Interface, pmt);
        if (FAILED(hr))
        {
#ifdef KSPROXY_TRACE
            WCHAR Buffer[100];
            swprintf(Buffer, L"COutputPin::CreatePinHandle failed with %lx\n", hr);
            OutputDebugStringW(Buffer);
#endif
            return hr;
        }

        if (!m_InterfaceHandler)
        {
            // now load the IKsInterfaceHandler plugin
            hr = CoCreateInstance(Interface->Set, NULL, CLSCTX_INPROC_SERVER, IID_IKsInterfaceHandler, (void**)&InterfaceHandler);
            if (FAILED(hr))
            {
                // failed to load interface handler plugin
                CoTaskMemFree(MediumList);
                CoTaskMemFree(InterfaceList);

#ifdef KSPROXY_TRACE
                WCHAR Buffer[100];
                swprintf(Buffer, L"COutputPin::CreatePin failed to create interface handler %lx\n", hr);
                OutputDebugStringW(Buffer);
#endif

                return hr;
            }

            // now set the pin
            hr = InterfaceHandler->KsSetPin((IKsPin*)this);
            if (FAILED(hr))
            {
                // failed to initialize interface handler plugin
#ifdef KSPROXY_TRACE
                WCHAR Buffer[100];
                swprintf(Buffer, L"COutputPin::CreatePin failed to initialize interface handler %lx\n", hr);
                OutputDebugStringW(Buffer);
#endif
                InterfaceHandler->Release();
                CoTaskMemFree(MediumList);
                CoTaskMemFree(InterfaceList);
                return hr;
            }

            // store interface handler
            m_InterfaceHandler = InterfaceHandler;
        }
    }
    else
    {
#ifdef KSPROXY_TRACE
        WCHAR Buffer[100];
        swprintf(Buffer, L"COutputPin::CreatePin unexpected communication %u %s\n", m_Communication, m_PinName);
        OutputDebugStringW(Buffer);
#endif

        hr = E_FAIL;
    }

    // free medium / interface / dataformat
    CoTaskMemFree(MediumList);
    CoTaskMemFree(InterfaceList);

#ifdef KSPROXY_TRACE
    WCHAR Buffer[100];
    swprintf(Buffer, L"COutputPin::CreatePin Result %lx\n", hr);
    OutputDebugStringW(Buffer);
#endif

    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::CreatePinHandle(
    PKSPIN_MEDIUM Medium,
    PKSPIN_INTERFACE Interface,
    const AM_MEDIA_TYPE *pmt)
{
    PKSPIN_CONNECT PinConnect;
    PKSDATAFORMAT DataFormat;
    ULONG Length;
    HRESULT hr;
    HANDLE hFilter;
    IKsObject * KsObjectParent;

    //KSALLOCATOR_FRAMING Framing;
    //KSPROPERTY Property;
    //ULONG BytesReturned;

    OutputDebugStringW(L"COutputPin::CreatePinHandle\n");

    if (m_hPin != INVALID_HANDLE_VALUE)
    {
        // pin already exists
        //CloseHandle(m_hPin);
        //m_hPin = INVALID_HANDLE_VALUE;
        OutputDebugStringW(L"COutputPin::CreatePinHandle pin already exists\n");
        return S_OK;
    }


    // calc format size
    Length = sizeof(KSPIN_CONNECT) + sizeof(KSDATAFORMAT) + pmt->cbFormat;

    // allocate pin connect
    PinConnect = (PKSPIN_CONNECT)CoTaskMemAlloc(Length);
    if (!PinConnect)
    {
        // failed
        OutputDebugStringW(L"COutputPin::CreatePinHandle out of memory\n");
        return E_OUTOFMEMORY;
    }
        OutputDebugStringW(L"COutputPin::CreatePinHandle copy pinconnect\n");
    // setup request
    CopyMemory(&PinConnect->Interface, Interface, sizeof(KSPIN_INTERFACE));
    CopyMemory(&PinConnect->Medium, Medium, sizeof(KSPIN_MEDIUM));
    PinConnect->PinId = m_PinId;
    PinConnect->PinToHandle = NULL;
    PinConnect->Priority.PriorityClass = KSPRIORITY_NORMAL;
    PinConnect->Priority.PrioritySubClass = KSPRIORITY_NORMAL;

    // get dataformat offset
    DataFormat = (PKSDATAFORMAT)(PinConnect + 1);
        OutputDebugStringW(L"COutputPin::CreatePinHandle copy format\n");
    // copy data format
    DataFormat->FormatSize = sizeof(KSDATAFORMAT) + pmt->cbFormat;
    DataFormat->Flags = 0;
    DataFormat->SampleSize = pmt->lSampleSize;
    DataFormat->Reserved = 0;
    CopyMemory(&DataFormat->MajorFormat, &pmt->majortype, sizeof(GUID));
    CopyMemory(&DataFormat->SubFormat,  &pmt->subtype, sizeof(GUID));
    CopyMemory(&DataFormat->Specifier, &pmt->formattype, sizeof(GUID));

    if (pmt->cbFormat)
    {
        // copy extended format
        WCHAR Buffer[100];
        swprintf(Buffer, L"COutputPin::CreatePinHandle copy format %p pbFormat %lu\n", pmt, pmt->cbFormat);
        OutputDebugStringW(Buffer);
        CopyMemory((DataFormat + 1), pmt->pbFormat, pmt->cbFormat);
    }

    // get IKsObject interface
    hr = m_ParentFilter->QueryInterface(IID_IKsObject, (LPVOID*)&KsObjectParent);
    if (FAILED(hr))
    {
        OutputDebugStringW(L"COutputPin::CreatePinHandle no IID_IKsObject interface\n");
        return hr;
    }

    // get parent filter handle
    hFilter = KsObjectParent->KsGetObjectHandle();

    // release IKsObject
    KsObjectParent->Release();

    if (!hFilter)
    {
        OutputDebugStringW(L"COutputPin::CreatePinHandle no filter handle\n");
        return E_HANDLE;
    }

    OutputDebugStringW(L"COutputPin::CreatePinHandle before creating pin\n");
    // create pin
    DWORD dwError = KsCreatePin(hFilter, PinConnect, GENERIC_READ, &m_hPin);

    if (dwError == ERROR_SUCCESS)
    {
        OutputDebugStringW(L"COutputPin::CreatePinHandle created pin\n");

        // store current interface / medium
        CopyMemory(&m_Medium, Medium, sizeof(KSPIN_MEDIUM));
        CopyMemory(&m_Interface, Interface, sizeof(KSPIN_INTERFACE));
        CopyMemory(&m_MediaFormat, pmt, sizeof(AM_MEDIA_TYPE));

#ifdef KSPROXY_TRACE
        LPOLESTR pMajor, pSub, pFormat;
        StringFromIID(m_MediaFormat.majortype, &pMajor);
        StringFromIID(m_MediaFormat.subtype , &pSub);
        StringFromIID(m_MediaFormat.formattype, &pFormat);
        WCHAR Buffer[200];
        swprintf(Buffer, L"COutputPin::CreatePinHandle Major %s SubType %s Format %s pbFormat %p cbFormat %u\n", pMajor, pSub, pFormat, pmt->pbFormat, pmt->cbFormat);
        CoTaskMemFree(pMajor);
        CoTaskMemFree(pSub);
        CoTaskMemFree(pFormat);
        OutputDebugStringW(Buffer);
#endif

        if (pmt->cbFormat)
        {
            m_MediaFormat.pbFormat = (BYTE*)CoTaskMemAlloc(pmt->cbFormat);
            if (!m_MediaFormat.pbFormat)
            {
                CoTaskMemFree(PinConnect);
                m_MediaFormat.pbFormat = NULL;
                m_MediaFormat.cbFormat = 0;
                return E_OUTOFMEMORY;
            }
            CopyMemory(m_MediaFormat.pbFormat, pmt->pbFormat, pmt->cbFormat);
        }
#if 0
        Property.Set = KSPROPSETID_Connection;
        Property.Id = KSPROPERTY_CONNECTION_ALLOCATORFRAMING;
        Property.Flags = KSPROPERTY_TYPE_GET;

        ZeroMemory(&Framing, sizeof(KSALLOCATOR_FRAMING));
        hr = KsProperty(&Property, sizeof(KSPROPERTY), (PVOID)&Framing, sizeof(KSALLOCATOR_FRAMING), &BytesReturned);
        if (SUCCEEDED(hr))
        {
            m_Properties.cbAlign = (Framing.FileAlignment + 1);
            m_Properties.cbBuffer = Framing.FrameSize;
            m_Properties.cbPrefix = 0; //FIXME
            m_Properties.cBuffers = Framing.Frames;
        }
        hr = S_OK;
#endif

        if (FAILED(InitializeIOThread()))
        {
            OutputDebugStringW(L"COutputPin::CreatePinHandle failed to initialize i/o thread\n");
        }

        //TODO
        // connect pin pipes

    }
    else
        OutputDebugStringW(L"COutputPin::CreatePinHandle failed to create pin\n");
    // free pin connect
     CoTaskMemFree(PinConnect);

    return hr;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::GetSupportedSets(
    LPGUID * pOutGuid,
    PULONG NumGuids)
{
    KSPROPERTY Property;
    LPGUID pGuid;
    ULONG NumProperty = 0;
    ULONG NumMethods = 0;
    ULONG NumEvents = 0;
    ULONG Length;
    ULONG BytesReturned;
    HRESULT hr;

    Property.Set = GUID_NULL;
    Property.Id = 0;
    Property.Flags = KSPROPERTY_TYPE_SETSUPPORT;

    KsSynchronousDeviceControl(m_hPin, IOCTL_KS_PROPERTY, (PVOID)&Property, sizeof(KSPROPERTY), NULL, 0, &NumProperty);
    KsSynchronousDeviceControl(m_hPin, IOCTL_KS_METHOD, (PVOID)&Property, sizeof(KSPROPERTY), NULL, 0, &NumMethods);
    KsSynchronousDeviceControl(m_hPin, IOCTL_KS_ENABLE_EVENT, (PVOID)&Property, sizeof(KSPROPERTY), NULL, 0, &NumEvents);

    Length = NumProperty + NumMethods + NumEvents;

    assert(Length);

    // allocate guid buffer
    pGuid = (LPGUID)CoTaskMemAlloc(Length);
    if (!pGuid)
    {
        // failed
        return E_OUTOFMEMORY;
    }

    NumProperty /= sizeof(GUID);
    NumMethods /= sizeof(GUID);
    NumEvents /= sizeof(GUID);

    // get all properties
    hr = KsSynchronousDeviceControl(m_hPin, IOCTL_KS_PROPERTY, (PVOID)&Property, sizeof(KSPROPERTY), (PVOID)pGuid, Length, &BytesReturned);
    if (FAILED(hr))
    {
        CoTaskMemFree(pGuid);
        return E_FAIL;
    }
    Length -= BytesReturned;

    // get all methods
    if (Length && NumMethods)
    {
        hr = KsSynchronousDeviceControl(m_hPin, IOCTL_KS_METHOD, (PVOID)&Property, sizeof(KSPROPERTY), (PVOID)&pGuid[NumProperty], Length, &BytesReturned);
        if (FAILED(hr))
        {
            CoTaskMemFree(pGuid);
            return E_FAIL;
        }
        Length -= BytesReturned;
    }

    // get all events
    if (Length && NumEvents)
    {
        hr = KsSynchronousDeviceControl(m_hPin, IOCTL_KS_ENABLE_EVENT, (PVOID)&Property, sizeof(KSPROPERTY), (PVOID)&pGuid[NumProperty+NumMethods], Length, &BytesReturned);
        if (FAILED(hr))
        {
            CoTaskMemFree(pGuid);
            return E_FAIL;
        }
        Length -= BytesReturned;
    }

#ifdef KSPROXY_TRACE
    WCHAR Buffer[200];
    swprintf(Buffer, L"NumProperty %lu NumMethods %lu NumEvents %lu\n", NumProperty, NumMethods, NumEvents);
    OutputDebugStringW(Buffer);
#endif

    *pOutGuid = pGuid;
    *NumGuids = NumProperty+NumEvents+NumMethods;
    return S_OK;
}

HRESULT
STDMETHODCALLTYPE
COutputPin::LoadProxyPlugins(
    LPGUID pGuids,
    ULONG NumGuids)
{
    ULONG Index;
    LPOLESTR pStr;
    HKEY hKey, hSubKey;
    HRESULT hr;
    IUnknown * pUnknown;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\MediaInterfaces", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        OutputDebugStringW(L"CKsProxy::LoadProxyPlugins failed to open MediaInterfaces key\n");
        return E_FAIL;
    }

    // enumerate all sets
    for(Index = 0; Index < NumGuids; Index++)
    {
        // convert to string
        hr = StringFromCLSID(pGuids[Index], &pStr);
        if (FAILED(hr))
            return E_FAIL;

        // now try open class key
        if (RegOpenKeyExW(hKey, pStr, 0, KEY_READ, &hSubKey) != ERROR_SUCCESS)
        {
            // no plugin for that set exists
            CoTaskMemFree(pStr);
            continue;
        }

        // try load plugin
        hr = CoCreateInstance(pGuids[Index], (IBaseFilter*)this, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pUnknown);
        if (SUCCEEDED(hr))
        {
            // store plugin
            m_Plugins.push_back(pUnknown);
        }
        // close key
        RegCloseKey(hSubKey);
    }

    // close media interfaces key
    RegCloseKey(hKey);
    return S_OK;
}


HRESULT
WINAPI
COutputPin::IoProcessRoutine()
{
    IMediaSample *Sample;
    LONG SampleCount;
    HRESULT hr;
    PKSSTREAM_SEGMENT * StreamSegment;
    HANDLE hEvent;
    IMediaSample ** Samples;
    LONG NumHandles;
    DWORD dwStatus;

#ifdef KSPROXY_TRACE
    WCHAR Buffer[200];
#endif

    NumHandles = m_Properties.cBuffers / 2;

    if (!NumHandles)
        NumHandles = 8;

    assert(NumHandles);

    //allocate stream segment array
    StreamSegment = (PKSSTREAM_SEGMENT*)CoTaskMemAlloc(sizeof(PVOID) * NumHandles);
    if (!StreamSegment)
    {
        OutputDebugStringW(L"COutputPin::IoProcessRoutine out of memory\n");
        return E_FAIL;
    }

    // allocate handle array
    Samples = (IMediaSample**)CoTaskMemAlloc(sizeof(IMediaSample*) * NumHandles);
    if (!Samples)
    {
        OutputDebugStringW(L"COutputPin::IoProcessRoutine out of memory\n");
        return E_FAIL;
    }

    // zero handles array
    ZeroMemory(StreamSegment, sizeof(PVOID) * NumHandles);
    ZeroMemory(Samples, sizeof(IMediaSample*) * NumHandles);

    // first wait for the start event to signal
    WaitForSingleObject(m_hStartEvent, INFINITE);

    m_IoCount = 0;

    assert(m_InterfaceHandler);
    do
    {
        if (m_StopInProgress)
        {
            // stop io thread
            break;
        }

        assert(m_State == KSSTATE_RUN);
        assert(m_MemAllocator);

        // get buffer
        hr = m_MemAllocator->GetBuffer(&Sample, NULL, NULL, AM_GBF_NOWAIT);

        if (FAILED(hr))
        {
            WaitForSingleObject(m_hBufferAvailable, INFINITE);
            // now retry again
            continue;
        }

        // fill buffer
        SampleCount = 1;
        Samples[m_IoCount] = Sample;

        Sample->SetTime(NULL, NULL);
        hr = m_InterfaceHandler->KsProcessMediaSamples(NULL, /* FIXME */
                                                       &Samples[m_IoCount],
                                                       &SampleCount,
                                                       KsIoOperation_Read,
                                                       &StreamSegment[m_IoCount]);
        if (FAILED(hr) || !StreamSegment)
        {
#ifdef KSPROXY_TRACE
            swprintf(Buffer, L"COutputPin::IoProcessRoutine KsProcessMediaSamples FAILED PinName %s hr %lx\n", m_PinName, hr);
            OutputDebugStringW(Buffer);
#endif
            break;
        }

        // interface handle should increment pending i/o count
        assert(m_IoCount >= 1);

        swprintf(Buffer, L"COutputPin::IoProcessRoutine m_IoCount %lu NumHandles %lu\n", m_IoCount, NumHandles);
        OutputDebugStringW(Buffer);

        if (m_IoCount != NumHandles)
            continue;

        // get completion handle
        hEvent = StreamSegment[0]->CompletionEvent;

        // wait for i/o completion
        dwStatus = WaitForSingleObject(hEvent, INFINITE);

        swprintf(Buffer, L"COutputPin::IoProcessRoutine dwStatus %lx Error %lx NumHandles %lu\n", dwStatus, GetLastError(), NumHandles);
        OutputDebugStringW(Buffer);

        // perform completion
        m_InterfaceHandler->KsCompleteIo(StreamSegment[0]);

        // close completion event
        CloseHandle(hEvent);

        if (SUCCEEDED(hr))
        {
            assert(m_MemInputPin);

            // now deliver the sample
            hr = m_MemInputPin->Receive(Samples[0]);

#ifdef KSPROXY_TRACE
            swprintf(Buffer, L"COutputPin::IoProcessRoutine PinName %s IMemInputPin::Receive hr %lx Sample %p m_MemAllocator %p\n", m_PinName, hr, Sample, m_MemAllocator);
            OutputDebugStringW(Buffer);
#endif

             if (FAILED(hr))
                 break;

            Sample = NULL;
        }

        //circular stream segment array
        RtlMoveMemory(StreamSegment, &StreamSegment[1], sizeof(PVOID) * (NumHandles - 1));
        RtlMoveMemory(Samples, &Samples[1], sizeof(IMediaSample*) * (NumHandles - 1));

    }while(TRUE);

    // signal end of i/o thread
    SetEvent(m_hStopEvent);

    m_IoThreadStarted = false;

    return NOERROR;
}

DWORD
WINAPI
COutputPin_IoThreadStartup(
    LPVOID lpParameter)
{
    COutputPin * Pin = (COutputPin*)lpParameter;
    assert(Pin);

    return Pin->IoProcessRoutine();
}


HRESULT
WINAPI
COutputPin::InitializeIOThread()
{
    HANDLE hThread;

    if (m_IoThreadStarted)
        return NOERROR;

    if (!m_hStartEvent)
        m_hStartEvent = CreateEventW(NULL, FALSE, FALSE, NULL);

    if (!m_hStartEvent)
        return E_OUTOFMEMORY;

    if (!m_hStopEvent)
        m_hStopEvent = CreateEventW(NULL, FALSE, FALSE, NULL);

    if (!m_hStopEvent)
        return E_OUTOFMEMORY;

    if (!m_hBufferAvailable)
        m_hBufferAvailable = CreateEventW(NULL, FALSE, FALSE, NULL);

    if (!m_hBufferAvailable)
        return E_OUTOFMEMORY;

    m_StopInProgress = false;
    m_IoThreadStarted = true;

    // now create the startup thread
    hThread = CreateThread(NULL, 0, COutputPin_IoThreadStartup, (LPVOID)this, 0, NULL);
    if (!hThread)
        return E_OUTOFMEMORY;


    // close thread handle
    CloseHandle(hThread);
    return NOERROR;
}

HRESULT
STDMETHODCALLTYPE
COutputPin_SetState(
    IPin * Pin,
    KSSTATE State)
{
    HRESULT hr = S_OK;
    KSPROPERTY Property;
    KSSTATE CurState;
    ULONG BytesReturned;
    COutputPin * pPin = (COutputPin*)Pin;

#ifdef KSPROXY_TRACE
    WCHAR Buffer[200];
#endif

    Property.Set = KSPROPSETID_Connection;
    Property.Id = KSPROPERTY_CONNECTION_STATE;
    Property.Flags = KSPROPERTY_TYPE_SET;

    EnterCriticalSection(&pPin->m_Lock);

    if (pPin->m_State <= State)
    {
        if (pPin->m_State == KSSTATE_STOP)
        {
            hr = pPin->InitializeIOThread();
            if (FAILED(hr))
            {
                // failed to initialize I/O thread
#ifdef KSPROXY_TRACE
                OutputDebugStringW(L"Failed to initialize I/O Thread\n");
#endif
                LeaveCriticalSection(&pPin->m_Lock);
                return hr;
            }
            CurState = KSSTATE_ACQUIRE;
            hr = pPin->KsProperty(&Property, sizeof(KSPROPERTY), &CurState, sizeof(KSSTATE), &BytesReturned);

#ifdef KSPROXY_TRACE
            swprintf(Buffer, L"COutputPin_SetState Setting State CurState: KSSTATE_STOP KSSTATE_ACQUIRE PinName %s hr %lx\n", pPin->m_PinName, hr);
            OutputDebugStringW(Buffer);
#endif

            if (FAILED(hr))
            {
                LeaveCriticalSection(&pPin->m_Lock);
                return hr;
            }

            pPin->m_State = CurState;

            if (pPin->m_State == State)
            {
                LeaveCriticalSection(&pPin->m_Lock);
                return hr;
            }
        }
        if (pPin->m_State == KSSTATE_ACQUIRE)
        {
            CurState = KSSTATE_PAUSE;
            hr = pPin->KsProperty(&Property, sizeof(KSPROPERTY), &CurState, sizeof(KSSTATE), &BytesReturned);

#ifdef KSPROXY_TRACE
            swprintf(Buffer, L"COutputPin_SetState Setting State CurState KSSTATE_ACQUIRE KSSTATE_PAUSE PinName %s hr %lx\n", pPin->m_PinName, hr);
            OutputDebugStringW(Buffer);
#endif

            if (FAILED(hr))
            {
                LeaveCriticalSection(&pPin->m_Lock);
                return hr;
            }

            pPin->m_State = CurState;

            if (pPin->m_State == State)
            {
                LeaveCriticalSection(&pPin->m_Lock);
                return hr;
            }
        }
        if (State == KSSTATE_RUN && pPin->m_State == KSSTATE_PAUSE)
        {
            CurState = KSSTATE_RUN;
            hr = pPin->KsProperty(&Property, sizeof(KSPROPERTY), &CurState, sizeof(KSSTATE), &BytesReturned);

#ifdef KSPROXY_TRACE
            swprintf(Buffer, L"COutputPin_SetState Setting State CurState: KSSTATE_PAUSE KSSTATE_RUN PinName %s hr %lx\n", pPin->m_PinName, hr);
            OutputDebugStringW(Buffer);
#endif

            if (SUCCEEDED(hr))
            {
                pPin->m_State = CurState;
                // signal start event
                SetEvent(pPin->m_hStartEvent);
            }
        }

        LeaveCriticalSection(&pPin->m_Lock);
        return hr;
    }
    else
    {
        if (pPin->m_State == KSSTATE_RUN)
        {
            // setting pending stop flag
            pPin->m_StopInProgress = true;

            // release any waiting threads
            SetEvent(pPin->m_hBufferAvailable);

            // wait until i/o thread is done
            WaitForSingleObject(pPin->m_hStopEvent, INFINITE);

            CurState = KSSTATE_PAUSE;
            hr = pPin->KsProperty(&Property, sizeof(KSPROPERTY), &CurState, sizeof(KSSTATE), &BytesReturned);

#ifdef KSPROXY_TRACE
            swprintf(Buffer, L"COutputPin_SetState Setting State CurState: KSSTATE_RUN KSSTATE_PAUSE PinName %s hr %lx\n", pPin->m_PinName, hr);
            OutputDebugStringW(Buffer);
#endif

            if (FAILED(hr))
            {
                LeaveCriticalSection(&pPin->m_Lock);
                return hr;
            }

            pPin->m_State = CurState;

            if (FAILED(hr))
            {
                LeaveCriticalSection(&pPin->m_Lock);
                return hr;
            }
        }
        if (pPin->m_State == KSSTATE_PAUSE)
        {
            CurState = KSSTATE_ACQUIRE;
            hr = pPin->KsProperty(&Property, sizeof(KSPROPERTY), &CurState, sizeof(KSSTATE), &BytesReturned);

#ifdef KSPROXY_TRACE
            swprintf(Buffer, L"COutputPin_SetState Setting State CurState: KSSTATE_PAUSE KSSTATE_ACQUIRE PinName %s hr %lx\n", pPin->m_PinName, hr);
            OutputDebugStringW(Buffer);
#endif

            if (FAILED(hr))
            {
                LeaveCriticalSection(&pPin->m_Lock);
                return hr;
            }

            pPin->m_State = CurState;

            if (pPin->m_State == State)
            {
                LeaveCriticalSection(&pPin->m_Lock);
                return hr;
            }
        }

        CloseHandle(pPin->m_hStopEvent);
        CloseHandle(pPin->m_hStartEvent);
        CloseHandle(pPin->m_hBufferAvailable);

        /* close event handles */
        pPin->m_hStopEvent = NULL;
        pPin->m_hStartEvent = NULL;
        pPin->m_hBufferAvailable = NULL;

        CurState = KSSTATE_STOP;
        hr = pPin->KsProperty(&Property, sizeof(KSPROPERTY), &CurState, sizeof(KSSTATE), &BytesReturned);

#ifdef KSPROXY_TRACE
        swprintf(Buffer, L"COutputPin_SetState Setting State CurState: KSSTATE_ACQUIRE KSSTATE_STOP PinName %s hr %lx\n", pPin->m_PinName, hr);
        OutputDebugStringW(Buffer);
#endif

        if (SUCCEEDED(hr))
        {
            // store state
            pPin->m_State = CurState;
        }

        // unset pending stop flag
        pPin->m_StopInProgress = false;

        LeaveCriticalSection(&pPin->m_Lock);
        return hr;
    }
}

HRESULT
WINAPI
COutputPin_Constructor(
    IBaseFilter * ParentFilter,
    LPCWSTR PinName,
    ULONG PinId,
    KSPIN_COMMUNICATION Communication,
    REFIID riid,
    LPVOID * ppv)
{
    COutputPin * handler = new COutputPin(ParentFilter, PinName, PinId, Communication);

    if (!handler)
        return E_OUTOFMEMORY;

    if (FAILED(handler->QueryInterface(riid, ppv)))
    {
        /* not supported */
        delete handler;
        return E_NOINTERFACE;
    }

    return S_OK;
}
