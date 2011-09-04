/*
 * Implementation of IWebBrowser interface for WebBrowser control
 *
 * Copyright 2001 John R. Sheets (for CodeWeavers)
 * Copyright 2005 Jacek Caban
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "wine/debug.h"
#include "shdocvw.h"
#include "exdispid.h"
#include "mshtml.h"

WINE_DEFAULT_DEBUG_CHANNEL(shdocvw);

/**********************************************************************
 * Implement the IWebBrowser interface
 */

#define WEBBROWSER_THIS(iface) DEFINE_THIS(WebBrowser, WebBrowser2, iface)

static HRESULT WINAPI WebBrowser_QueryInterface(IWebBrowser2 *iface, REFIID riid, LPVOID *ppv)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    if (ppv == NULL)
        return E_POINTER;
    *ppv = NULL;

    if(IsEqualGUID(&IID_IUnknown, riid)) {
        TRACE("(%p)->(IID_IUnknown %p)\n", This, ppv);
        *ppv = WEBBROWSER(This);
    }else if(IsEqualGUID(&IID_IDispatch, riid)) {
        TRACE("(%p)->(IID_IDispatch %p)\n", This, ppv);
        *ppv = WEBBROWSER(This);
    }else if(IsEqualGUID(&IID_IWebBrowser, riid)) {
        TRACE("(%p)->(IID_IWebBrowser %p)\n", This, ppv);
        *ppv = WEBBROWSER(This);
    }else if(IsEqualGUID(&IID_IWebBrowserApp, riid)) {
        TRACE("(%p)->(IID_IWebBrowserApp %p)\n", This, ppv);
        *ppv = WEBBROWSER(This);
    }else if(IsEqualGUID(&IID_IWebBrowser2, riid)) {
        TRACE("(%p)->(IID_IWebBrowser2 %p)\n", This, ppv);
        *ppv = WEBBROWSER(This);
    }else if(IsEqualGUID(&IID_IOleObject, riid)) {
        TRACE("(%p)->(IID_IOleObject %p)\n", This, ppv);
        *ppv = OLEOBJ(This);
    }else if(IsEqualGUID(&IID_IOleWindow, riid)) {
        TRACE("(%p)->(IID_IOleWindow %p)\n", This, ppv);
        *ppv = INPLACEOBJ(This);
    }else if(IsEqualGUID (&IID_IOleInPlaceObject, riid)) {
        TRACE("(%p)->(IID_IOleInPlaceObject %p)\n", This, ppv);
        *ppv = INPLACEOBJ(This);
    }else if(IsEqualGUID(&IID_IOleControl, riid)) {
        TRACE("(%p)->(IID_IOleControl %p)\n", This, ppv);
        *ppv = CONTROL(This);
    }else if(IsEqualGUID(&IID_IPersist, riid)) {
        TRACE("(%p)->(IID_IPersist %p)\n", This, ppv);
        *ppv = PERSTORAGE(This);
    }else if(IsEqualGUID(&IID_IPersistStorage, riid)) {
        TRACE("(%p)->(IID_IPersistStorage %p)\n", This, ppv);
        *ppv = PERSTORAGE(This);
    }else if(IsEqualGUID(&IID_IPersistMemory, riid)) {
        TRACE("(%p)->(IID_IPersistStorage %p)\n", This, ppv);
        *ppv = PERMEMORY(This);
    }else if(IsEqualGUID (&IID_IPersistStreamInit, riid)) {
        TRACE("(%p)->(IID_IPersistStreamInit %p)\n", This, ppv);
        *ppv = PERSTRINIT(This);
    }else if(IsEqualGUID(&IID_IProvideClassInfo, riid)) {
        TRACE("(%p)->(IID_IProvideClassInfo %p)\n", This, ppv);
        *ppv = CLASSINFO(This);
    }else if(IsEqualGUID(&IID_IProvideClassInfo2, riid)) {
        TRACE("(%p)->(IID_IProvideClassInfo2 %p)\n", This, ppv);
        *ppv = CLASSINFO(This);
    }else if(IsEqualGUID(&IID_IConnectionPointContainer, riid)) {
        TRACE("(%p)->(IID_IConnectionPointContainer %p)\n", This, ppv);
        *ppv = CONPTCONT(&This->doc_host.cps);
    }else if(IsEqualGUID(&IID_IViewObject, riid)) {
        TRACE("(%p)->(IID_IViewObject %p)\n", This, ppv);
        *ppv = VIEWOBJ(This);
    }else if(IsEqualGUID(&IID_IViewObject2, riid)) {
        TRACE("(%p)->(IID_IViewObject2 %p)\n", This, ppv);
        *ppv = VIEWOBJ2(This);
    }else if(IsEqualGUID(&IID_IOleInPlaceActiveObject, riid)) {
        TRACE("(%p)->(IID_IOleInPlaceActiveObject %p)\n", This, ppv);
        *ppv = ACTIVEOBJ(This);
    }else if(IsEqualGUID(&IID_IOleCommandTarget, riid)) {
        TRACE("(%p)->(IID_IOleCommandTarget %p)\n", This, ppv);
        *ppv = OLECMD(This);
    }else if(IsEqualGUID(&IID_IServiceProvider, riid)) {
        *ppv = SERVPROV(This);
        TRACE("(%p)->(IID_IServiceProvider %p)\n", This, ppv);
    }else if(IsEqualGUID(&IID_IDataObject, riid)) {
        *ppv = DATAOBJECT(This);
        TRACE("(%p)->(IID_IDataObject %p)\n", This, ppv);
    }else if(IsEqualGUID(&IID_IQuickActivate, riid)) {
        TRACE("(%p)->(IID_IQuickActivate %p) returning NULL\n", This, ppv);
        return E_NOINTERFACE;
    }else if(IsEqualGUID(&IID_IRunnableObject, riid)) {
        TRACE("(%p)->(IID_IRunnableObject %p) returning NULL\n", This, ppv);
        return E_NOINTERFACE;
    }else if(IsEqualGUID(&IID_IPerPropertyBrowsing, riid)) {
        TRACE("(%p)->(IID_IPerPropertyBrowsing %p) returning NULL\n", This, ppv);
        return E_NOINTERFACE;
    }else if(IsEqualGUID(&IID_IOleCache, riid)) {
        TRACE("(%p)->(IID_IOleCache %p) returning NULL\n", This, ppv);
        return E_NOINTERFACE;
    }else if(IsEqualGUID(&IID_IOleInPlaceSite, riid)) {
        TRACE("(%p)->(IID_IOleInPlaceSite %p) returning NULL\n", This, ppv);
        return E_NOINTERFACE;
    }else if(IsEqualGUID(&IID_IObjectWithSite, riid)) {
        TRACE("(%p)->(IID_IObjectWithSite %p) returning NULL\n", This, ppv);
        return E_NOINTERFACE;
    }else if(IsEqualGUID(&IID_IViewObjectEx, riid)) {
        TRACE("(%p)->(IID_IViewObjectEx %p) returning NULL\n", This, ppv);
        return E_NOINTERFACE;
    }else if(IsEqualGUID(&IID_IOleLink, riid)) {
        TRACE("(%p)->(IID_IOleLink %p) returning NULL\n", This, ppv);
        return E_NOINTERFACE;
    }else if(IsEqualGUID(&IID_IMarshal, riid)) {
        TRACE("(%p)->(IID_IMarshal %p) returning NULL\n", This, ppv);
        return E_NOINTERFACE;
    }else if(IsEqualGUID(&IID_IStdMarshalInfo, riid)) {
        TRACE("(%p)->(IID_IStdMarshalInfo %p) returning NULL\n", This, ppv);
        return E_NOINTERFACE;
    }else if(HlinkFrame_QI(&This->hlink_frame, riid, ppv)) {
        return S_OK;
    }

    if(*ppv) {
        IUnknown_AddRef((IUnknown*)*ppv);
        return S_OK;
    }

    FIXME("(%p)->(%s %p) interface not supported\n", This, debugstr_guid(riid), ppv);
    return E_NOINTERFACE;
}

static ULONG WINAPI WebBrowser_AddRef(IWebBrowser2 *iface)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    LONG ref = InterlockedIncrement(&This->ref);
    TRACE("(%p) ref=%d\n", This, ref);
    return ref;
}

static ULONG WINAPI WebBrowser_Release(IWebBrowser2 *iface)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    LONG ref = InterlockedDecrement(&This->ref);

    TRACE("(%p) ref=%d\n", This, ref);

    if(!ref) {
        if(This->doc_host.document)
            IUnknown_Release(This->doc_host.document);

        DocHost_Release(&This->doc_host);

        WebBrowser_OleObject_Destroy(This);

        heap_free(This);
        SHDOCVW_UnlockModule();
    }

    return ref;
}

/* IDispatch methods */
static HRESULT WINAPI WebBrowser_GetTypeInfoCount(IWebBrowser2 *iface, UINT *pctinfo)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, pctinfo);

    *pctinfo = 1;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_GetTypeInfo(IWebBrowser2 *iface, UINT iTInfo, LCID lcid,
                                     LPTYPEINFO *ppTInfo)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    ITypeInfo *typeinfo;
    HRESULT hres;

    TRACE("(%p)->(%d %d %p)\n", This, iTInfo, lcid, ppTInfo);

    hres = get_typeinfo(&typeinfo);
    if(FAILED(hres))
        return hres;

    ITypeInfo_AddRef(typeinfo);
    *ppTInfo = typeinfo;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_GetIDsOfNames(IWebBrowser2 *iface, REFIID riid,
                                       LPOLESTR *rgszNames, UINT cNames,
                                       LCID lcid, DISPID *rgDispId)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    ITypeInfo *typeinfo;
    HRESULT hres;

    TRACE("(%p)->(%s %p %d %d %p)\n", This, debugstr_guid(riid), rgszNames, cNames,
          lcid, rgDispId);

    hres = get_typeinfo(&typeinfo);
    if(FAILED(hres))
        return hres;

    return ITypeInfo_GetIDsOfNames(typeinfo, rgszNames, cNames, rgDispId);
}

static HRESULT WINAPI WebBrowser_Invoke(IWebBrowser2 *iface, DISPID dispIdMember,
                                REFIID riid, LCID lcid, WORD wFlags,
                                DISPPARAMS *pDispParams, VARIANT *pVarResult,
                                EXCEPINFO *pExepInfo, UINT *puArgErr)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    ITypeInfo *typeinfo;
    HRESULT hres;

    TRACE("(%p)->(%d %s %d %08x %p %p %p %p)\n", This, dispIdMember, debugstr_guid(riid),
            lcid, wFlags, pDispParams, pVarResult, pExepInfo, puArgErr);

    hres = get_typeinfo(&typeinfo);
    if(FAILED(hres))
        return hres;

    return ITypeInfo_Invoke(typeinfo, WEBBROWSER2(This), dispIdMember, wFlags, pDispParams,
                            pVarResult, pExepInfo, puArgErr);
}

/* IWebBrowser methods */
static HRESULT WINAPI WebBrowser_GoBack(IWebBrowser2 *iface)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_GoForward(IWebBrowser2 *iface)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_GoHome(IWebBrowser2 *iface)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    TRACE("(%p)\n", This);
    return go_home(&This->doc_host);
}

static HRESULT WINAPI WebBrowser_GoSearch(IWebBrowser2 *iface)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_Navigate(IWebBrowser2 *iface, BSTR szUrl,
                                  VARIANT *Flags, VARIANT *TargetFrameName,
                                  VARIANT *PostData, VARIANT *Headers)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%s %s %s %s %s)\n", This, debugstr_w(szUrl), debugstr_variant(Flags),
          debugstr_variant(TargetFrameName), debugstr_variant(PostData),
          debugstr_variant(Headers));

    return navigate_url(&This->doc_host, szUrl, Flags, TargetFrameName, PostData, Headers);
}

static HRESULT WINAPI WebBrowser_Refresh(IWebBrowser2 *iface)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_Refresh2(IWebBrowser2 *iface, VARIANT *Level)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%s)\n", This, debugstr_variant(Level));
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_Stop(IWebBrowser2 *iface)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)\n", This);
    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_Application(IWebBrowser2 *iface, IDispatch **ppDisp)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, ppDisp);

    if(!ppDisp)
        return E_POINTER;

    *ppDisp = (IDispatch*)WEBBROWSER2(This);
    IDispatch_AddRef(*ppDisp);
    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_Parent(IWebBrowser2 *iface, IDispatch **ppDisp)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%p)\n", This, ppDisp);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_get_Container(IWebBrowser2 *iface, IDispatch **ppDisp)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%p)\n", This, ppDisp);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_get_Document(IWebBrowser2 *iface, IDispatch **ppDisp)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    IDispatch *disp = NULL;

    TRACE("(%p)->(%p)\n", This, ppDisp);

    if(This->doc_host.document) {
        HRESULT hres;

        hres = IUnknown_QueryInterface(This->doc_host.document, &IID_IDispatch, (void**)&disp);
        if(SUCCEEDED(hres)) {
            IDispatch *html_doc;

            /* Some broken apps cast returned IDispatch to IHTMLDocument2
             * without QueryInterface call */
            hres = IDispatch_QueryInterface(disp, &IID_IHTMLDocument2, (void**)&html_doc);
            if(SUCCEEDED(hres)) {
                IDispatch_Release(disp);
                disp = html_doc;
            }
        }
    }

    *ppDisp = disp;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_TopLevelContainer(IWebBrowser2 *iface, VARIANT_BOOL *pBool)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%p)\n", This, pBool);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_get_Type(IWebBrowser2 *iface, BSTR *Type)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%p)\n", This, Type);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_get_Left(IWebBrowser2 *iface, LONG *pl)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, pl);

    *pl = This->pos_rect.left;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_Left(IWebBrowser2 *iface, LONG Left)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    RECT rect;

    TRACE("(%p)->(%d)\n", This, Left);

    if(!This->inplace)
        return E_UNEXPECTED;

    rect = This->pos_rect;
    rect.left = Left;

    /* We don't really change the window position here.
     * We just notify the embedder that he should do so. */
    return IOleInPlaceSite_OnPosRectChange(This->inplace, &rect);
}

static HRESULT WINAPI WebBrowser_get_Top(IWebBrowser2 *iface, LONG *pl)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, pl);

    *pl = This->pos_rect.top;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_Top(IWebBrowser2 *iface, LONG Top)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    RECT rect;

    TRACE("(%p)->(%d)\n", This, Top);

    if(!This->inplace)
        return E_UNEXPECTED;

    rect = This->pos_rect;
    rect.top = Top;

    /* We don't really change the window position here.
     * We just notify the embedder that he should do so. */
    return IOleInPlaceSite_OnPosRectChange(This->inplace, &rect);
}

static HRESULT WINAPI WebBrowser_get_Width(IWebBrowser2 *iface, LONG *pl)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, pl);

    *pl = This->pos_rect.right - This->pos_rect.left;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_Width(IWebBrowser2 *iface, LONG Width)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    RECT rect;

    TRACE("(%p)->(%d)\n", This, Width);

    if(!This->inplace)
        return E_UNEXPECTED;

    rect = This->pos_rect;
    rect.right = rect.left+Width;

    /* We don't really change the window size here.
     * We just notify the embedder that he should do so. */
   return IOleInPlaceSite_OnPosRectChange(This->inplace, &rect);
}

static HRESULT WINAPI WebBrowser_get_Height(IWebBrowser2 *iface, LONG *pl)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, pl);

    *pl = This->pos_rect.bottom - This->pos_rect.top;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_Height(IWebBrowser2 *iface, LONG Height)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    RECT rect;

    TRACE("(%p)->(%d)\n", This, Height);

    if(!This->inplace)
        return E_UNEXPECTED;

    rect = This->pos_rect;
    rect.bottom = rect.top+Height;

    /* We don't really change the window size here.
     * We just notify the embedder that he should do so. */
    return IOleInPlaceSite_OnPosRectChange(This->inplace, &rect);
}

static HRESULT WINAPI WebBrowser_get_LocationName(IWebBrowser2 *iface, BSTR *LocationName)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%p)\n", This, LocationName);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_get_LocationURL(IWebBrowser2 *iface, BSTR *LocationURL)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    FIXME("(%p)->(%p)\n", This, LocationURL);

    if(!This->doc_host.url) {
        static const WCHAR null_char = 0;
        *LocationURL = SysAllocString(&null_char);
        return S_FALSE;
    }

    *LocationURL = SysAllocString(This->doc_host.url);
    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_Busy(IWebBrowser2 *iface, VARIANT_BOOL *pBool)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, pBool);

    *pBool = This->doc_host.busy;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_Quit(IWebBrowser2 *iface)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)\n", This);

    /* It's a InternetExplorer specific method, we have nothing to do here. */
    return E_FAIL;
}

static HRESULT WINAPI WebBrowser_ClientToWindow(IWebBrowser2 *iface, int *pcx, int *pcy)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%p %p)\n", This, pcx, pcy);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_PutProperty(IWebBrowser2 *iface, BSTR szProperty, VARIANT vtValue)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%s %s)\n", This, debugstr_w(szProperty), debugstr_variant(&vtValue));
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_GetProperty(IWebBrowser2 *iface, BSTR szProperty, VARIANT *pvtValue)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%s %s)\n", This, debugstr_w(szProperty), debugstr_variant(pvtValue));
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_get_Name(IWebBrowser2 *iface, BSTR *Name)
{
    static const WCHAR sName[] = {'M','i','c','r','o','s','o','f','t',' ','W','e','b',
                                  ' ','B','r','o','w','s','e','r',' ','C','o','n','t','r','o','l',0};
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, Name);

    *Name = SysAllocString(sName);

    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_HWND(IWebBrowser2 *iface, LONG *pHWND)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, pHWND);

    /* WebBrowser control never has a frame window (in opposition to InternetExplorer) */
    *pHWND = 0;
    return E_FAIL;
}

static HRESULT WINAPI WebBrowser_get_FullName(IWebBrowser2 *iface, BSTR *FullName)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%p)\n", This, FullName);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_get_Path(IWebBrowser2 *iface, BSTR *Path)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%p)\n", This, Path);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_get_Visible(IWebBrowser2 *iface, VARIANT_BOOL *pBool)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, pBool);

    *pBool = This->visible;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_Visible(IWebBrowser2 *iface, VARIANT_BOOL Value)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    VARIANTARG arg;
    DISPPARAMS dispparams = {&arg, NULL, 1, 0};

    TRACE("(%p)->(%x)\n", This, Value);

    This->visible = Value;

    V_VT(&arg) = VT_BOOL;
    V_BOOL(&arg) = Value;
    call_sink(This->doc_host.cps.wbe2, DISPID_ONVISIBLE, &dispparams);

    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_StatusBar(IWebBrowser2 *iface, VARIANT_BOOL *pBool)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, pBool);

    *pBool = This->status_bar;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_StatusBar(IWebBrowser2 *iface, VARIANT_BOOL Value)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    VARIANTARG arg;
    DISPPARAMS dispparams = {&arg, NULL, 1, 0};

    TRACE("(%p)->(%x)\n", This, Value);

    This->status_bar = Value ? VARIANT_TRUE : VARIANT_FALSE;

    /* In opposition to InternetExplorer, all we should do here is
     * inform the embedder about the status bar change. */

    V_VT(&arg) = VT_BOOL;
    V_BOOL(&arg) = Value;
    call_sink(This->doc_host.cps.wbe2, DISPID_ONSTATUSBAR, &dispparams);

    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_StatusText(IWebBrowser2 *iface, BSTR *StatusText)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%p)\n", This, StatusText);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_put_StatusText(IWebBrowser2 *iface, BSTR StatusText)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%s)\n", This, debugstr_w(StatusText));
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_get_ToolBar(IWebBrowser2 *iface, int *Value)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, Value);

    *Value = This->tool_bar;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_ToolBar(IWebBrowser2 *iface, int Value)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    VARIANTARG arg;
    DISPPARAMS dispparams = {&arg, NULL, 1, 0};

    TRACE("(%p)->(%x)\n", This, Value);

    This->tool_bar = Value ? VARIANT_TRUE : VARIANT_FALSE;

    /* In opposition to InternetExplorer, all we should do here is
     * inform the embedder about the tool bar change. */

    V_VT(&arg) = VT_BOOL;
    V_BOOL(&arg) = This->tool_bar;
    call_sink(This->doc_host.cps.wbe2, DISPID_ONTOOLBAR, &dispparams);

    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_MenuBar(IWebBrowser2 *iface, VARIANT_BOOL *Value)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, Value);

    *Value = This->menu_bar;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_MenuBar(IWebBrowser2 *iface, VARIANT_BOOL Value)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    VARIANTARG arg;
    DISPPARAMS dispparams = {&arg, NULL, 1, 0};

    TRACE("(%p)->(%x)\n", This, Value);

    This->menu_bar = Value ? VARIANT_TRUE : VARIANT_FALSE;

    /* In opposition to InternetExplorer, all we should do here is
     * inform the embedder about the menu bar change. */

    V_VT(&arg) = VT_BOOL;
    V_BOOL(&arg) = Value;
    call_sink(This->doc_host.cps.wbe2, DISPID_ONMENUBAR, &dispparams);

    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_FullScreen(IWebBrowser2 *iface, VARIANT_BOOL *pbFullScreen)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, pbFullScreen);

    *pbFullScreen = This->full_screen;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_FullScreen(IWebBrowser2 *iface, VARIANT_BOOL bFullScreen)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    VARIANTARG arg;
    DISPPARAMS dispparams = {&arg, NULL, 1, 0};

    /* In opposition to InternetExplorer, all we should do here is
     * inform the embedder about the fullscreen change. */

    TRACE("(%p)->(%x)\n", This, bFullScreen);

    This->full_screen = bFullScreen ? VARIANT_TRUE : VARIANT_FALSE;

    V_VT(&arg) = VT_BOOL;
    V_BOOL(&arg) = bFullScreen;
    call_sink(This->doc_host.cps.wbe2, DISPID_ONFULLSCREEN, &dispparams);

    return S_OK;
}

static HRESULT WINAPI WebBrowser_Navigate2(IWebBrowser2 *iface, VARIANT *URL, VARIANT *Flags,
        VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    LPCWSTR url;

    TRACE("(%p)->(%s %s %s %s %s)\n", This, debugstr_variant(URL), debugstr_variant(Flags),
          debugstr_variant(TargetFrameName), debugstr_variant(PostData), debugstr_variant(Headers));

    if(!This->client)
        return E_FAIL;

    if(!URL)
        return S_OK;

    switch (V_VT(URL))
    {
    case VT_BSTR:
        url = V_BSTR(URL);
        break;
    case VT_BSTR|VT_BYREF:
        url = *V_BSTRREF(URL);
        break;
    default:
        FIXME("Unsupported V_VT(URL) %d\n", V_VT(URL));
        return E_INVALIDARG;
    }

    return navigate_url(&This->doc_host, url, Flags, TargetFrameName, PostData, Headers);
}

static HRESULT WINAPI WebBrowser_QueryStatusWB(IWebBrowser2 *iface, OLECMDID cmdID, OLECMDF *pcmdf)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%d %p)\n", This, cmdID, pcmdf);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_ExecWB(IWebBrowser2 *iface, OLECMDID cmdID,
        OLECMDEXECOPT cmdexecopt, VARIANT *pvaIn, VARIANT *pvaOut)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%d %d %s %p)\n", This, cmdID, cmdexecopt, debugstr_variant(pvaIn), pvaOut);
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_ShowBrowserBar(IWebBrowser2 *iface, VARIANT *pvaClsid,
        VARIANT *pvarShow, VARIANT *pvarSize)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%s %s %s)\n", This, debugstr_variant(pvaClsid), debugstr_variant(pvarShow),
          debugstr_variant(pvarSize));
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_get_ReadyState(IWebBrowser2 *iface, READYSTATE *lpReadyState)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, lpReadyState);

    *lpReadyState = This->doc_host.ready_state;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_Offline(IWebBrowser2 *iface, VARIANT_BOOL *pbOffline)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, pbOffline);

    *pbOffline = This->doc_host.offline;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_Offline(IWebBrowser2 *iface, VARIANT_BOOL bOffline)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%x)\n", This, bOffline);

    This->doc_host.offline = bOffline ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_Silent(IWebBrowser2 *iface, VARIANT_BOOL *pbSilent)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, pbSilent);

    *pbSilent = This->doc_host.silent;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_Silent(IWebBrowser2 *iface, VARIANT_BOOL bSilent)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%x)\n", This, bSilent);

    This->doc_host.silent = bSilent ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_RegisterAsBrowser(IWebBrowser2 *iface,
        VARIANT_BOOL *pbRegister)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    FIXME("(%p)->(%p)\n", This, pbRegister);

    *pbRegister = This->register_browser;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_RegisterAsBrowser(IWebBrowser2 *iface,
        VARIANT_BOOL bRegister)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    FIXME("(%p)->(%x)\n", This, bRegister);

    This->register_browser = bRegister ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_RegisterAsDropTarget(IWebBrowser2 *iface,
        VARIANT_BOOL *pbRegister)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%p)\n", This, pbRegister);
    *pbRegister=0;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_RegisterAsDropTarget(IWebBrowser2 *iface,
        VARIANT_BOOL bRegister)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    FIXME("(%p)->(%x)\n", This, bRegister);
    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_TheaterMode(IWebBrowser2 *iface, VARIANT_BOOL *pbRegister)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, pbRegister);

    *pbRegister = This->theater_mode;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_TheaterMode(IWebBrowser2 *iface, VARIANT_BOOL bRegister)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    VARIANTARG arg;
    DISPPARAMS dispparams = {&arg, NULL, 1, 0};

    TRACE("(%p)->(%x)\n", This, bRegister);

    This->theater_mode = bRegister ? VARIANT_TRUE : VARIANT_FALSE;

    /* In opposition to InternetExplorer, all we should do here is
     * inform the embedder about the theater mode change. */

    V_VT(&arg) = VT_BOOL;
    V_BOOL(&arg) = bRegister;
    call_sink(This->doc_host.cps.wbe2, DISPID_ONTHEATERMODE, &dispparams);

    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_AddressBar(IWebBrowser2 *iface, VARIANT_BOOL *Value)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, Value);

    *Value = This->address_bar;
    return S_OK;
}

static HRESULT WINAPI WebBrowser_put_AddressBar(IWebBrowser2 *iface, VARIANT_BOOL Value)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    VARIANTARG arg;
    DISPPARAMS dispparams = {&arg, NULL, 1, 0};

    TRACE("(%p)->(%x)\n", This, Value);

    This->address_bar = Value ? VARIANT_TRUE : VARIANT_FALSE;

    /* In opposition to InternetExplorer, all we should do here is
     * inform the embedder about the address bar change. */

    V_VT(&arg) = VT_BOOL;
    V_BOOL(&arg) = Value;
    call_sink(This->doc_host.cps.wbe2, DISPID_ONADDRESSBAR, &dispparams);

    return S_OK;
}

static HRESULT WINAPI WebBrowser_get_Resizable(IWebBrowser2 *iface, VARIANT_BOOL *Value)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);

    TRACE("(%p)->(%p)\n", This, Value);

    /* It's InternetExplorer object's method. We have nothing to do here. */
    return E_NOTIMPL;
}

static HRESULT WINAPI WebBrowser_put_Resizable(IWebBrowser2 *iface, VARIANT_BOOL Value)
{
    WebBrowser *This = WEBBROWSER_THIS(iface);
    VARIANTARG arg;
    DISPPARAMS dispparams = {&arg, NULL, 1, 0};

    TRACE("(%p)->(%x)\n", This, Value);

    /* In opposition to InternetExplorer, all we should do here is
     * inform the embedder about the resizable change. */

    V_VT(&arg) = VT_BOOL;
    V_BOOL(&arg) = Value;
    call_sink(This->doc_host.cps.wbe2, DISPID_WINDOWSETRESIZABLE, &dispparams);

    return S_OK;
}

#undef WEBBROWSER_THIS

static const IWebBrowser2Vtbl WebBrowser2Vtbl =
{
    WebBrowser_QueryInterface,
    WebBrowser_AddRef,
    WebBrowser_Release,
    WebBrowser_GetTypeInfoCount,
    WebBrowser_GetTypeInfo,
    WebBrowser_GetIDsOfNames,
    WebBrowser_Invoke,
    WebBrowser_GoBack,
    WebBrowser_GoForward,
    WebBrowser_GoHome,
    WebBrowser_GoSearch,
    WebBrowser_Navigate,
    WebBrowser_Refresh,
    WebBrowser_Refresh2,
    WebBrowser_Stop,
    WebBrowser_get_Application,
    WebBrowser_get_Parent,
    WebBrowser_get_Container,
    WebBrowser_get_Document,
    WebBrowser_get_TopLevelContainer,
    WebBrowser_get_Type,
    WebBrowser_get_Left,
    WebBrowser_put_Left,
    WebBrowser_get_Top,
    WebBrowser_put_Top,
    WebBrowser_get_Width,
    WebBrowser_put_Width,
    WebBrowser_get_Height,
    WebBrowser_put_Height,
    WebBrowser_get_LocationName,
    WebBrowser_get_LocationURL,
    WebBrowser_get_Busy,
    WebBrowser_Quit,
    WebBrowser_ClientToWindow,
    WebBrowser_PutProperty,
    WebBrowser_GetProperty,
    WebBrowser_get_Name,
    WebBrowser_get_HWND,
    WebBrowser_get_FullName,
    WebBrowser_get_Path,
    WebBrowser_get_Visible,
    WebBrowser_put_Visible,
    WebBrowser_get_StatusBar,
    WebBrowser_put_StatusBar,
    WebBrowser_get_StatusText,
    WebBrowser_put_StatusText,
    WebBrowser_get_ToolBar,
    WebBrowser_put_ToolBar,
    WebBrowser_get_MenuBar,
    WebBrowser_put_MenuBar,
    WebBrowser_get_FullScreen,
    WebBrowser_put_FullScreen,
    WebBrowser_Navigate2,
    WebBrowser_QueryStatusWB,
    WebBrowser_ExecWB,
    WebBrowser_ShowBrowserBar,
    WebBrowser_get_ReadyState,
    WebBrowser_get_Offline,
    WebBrowser_put_Offline,
    WebBrowser_get_Silent,
    WebBrowser_put_Silent,
    WebBrowser_get_RegisterAsBrowser,
    WebBrowser_put_RegisterAsBrowser,
    WebBrowser_get_RegisterAsDropTarget,
    WebBrowser_put_RegisterAsDropTarget,
    WebBrowser_get_TheaterMode,
    WebBrowser_put_TheaterMode,
    WebBrowser_get_AddressBar,
    WebBrowser_put_AddressBar,
    WebBrowser_get_Resizable,
    WebBrowser_put_Resizable
};

#define SERVPROV_THIS(iface) DEFINE_THIS(WebBrowser, OleObject, iface)
/*
 *  IServiceProvider interface.
 */
static HRESULT WINAPI WebBrowser_IServiceProvider_QueryInterface(IServiceProvider *iface,
            REFIID riid, LPVOID *ppv)
{
    WebBrowser *This = SERVPROV_THIS(iface);

    if (ppv == NULL)
        return E_POINTER;
    *ppv = NULL;

    if(IsEqualGUID(&IID_IUnknown, riid)) {
        *ppv = WEBBROWSER(This);
        TRACE("(%p)->(IID_IUnknown %p)\n", This, ppv);
    }else if(IsEqualGUID(&IID_IServiceProvider, riid)) {
        *ppv = WEBBROWSER(This);
        TRACE("(%p)->(IID_IServiceProvider %p)\n", This, ppv);
    }

    if(*ppv) {
        IUnknown_AddRef((IUnknown*)*ppv);
        return S_OK;
    }

    FIXME("(%p)->(%s %p) interface not supported\n", This, debugstr_guid(riid), ppv);
    return E_NOINTERFACE;
}

static ULONG WINAPI WebBrowser_IServiceProvider_AddRef(IServiceProvider *iface)
{
    WebBrowser *This = SERVPROV_THIS(iface);
    return IWebBrowser_AddRef(WEBBROWSER(This));
}

static ULONG WINAPI WebBrowser_IServiceProvider_Release(IServiceProvider *iface)
{
    WebBrowser *This = SERVPROV_THIS(iface);
    return IWebBrowser_Release(WEBBROWSER(This));
}

static HRESULT STDMETHODCALLTYPE WebBrowser_IServiceProvider_QueryService(IServiceProvider *iface,
            REFGUID guidService, REFIID riid, void **ppv)
{
    WebBrowser *This = SERVPROV_THIS(iface);
    static const IID IID_IBrowserService2 =
        {0x68BD21CC,0x438B,0x11d2,{0xA5,0x60,0x00,0xA0,0xC,0x2D,0xBF,0xE8}};

    if(*ppv)
        ppv = NULL;

    if(IsEqualGUID(&IID_IBrowserService2, riid)) {
        TRACE("(%p)->(IID_IBrowserService2 return E_FAIL)\n", This);
        return E_FAIL;
    }

    FIXME("(%p)->(%s, %s %p)\n", This, debugstr_guid(guidService), debugstr_guid(riid), ppv);

    return E_NOINTERFACE;
}

#undef SERVPROV_THIS

static const IServiceProviderVtbl ServiceProviderVtbl =
{
    WebBrowser_IServiceProvider_QueryInterface,
    WebBrowser_IServiceProvider_AddRef,
    WebBrowser_IServiceProvider_Release,
    WebBrowser_IServiceProvider_QueryService
};

#define DOCHOST_THIS(iface) DEFINE_THIS2(WebBrowser,doc_host,iface)

static void WINAPI DocHostContainer_GetDocObjRect(DocHost* This, RECT* rc)
{
    GetClientRect(This->frame_hwnd, rc);
}

static HRESULT WINAPI DocHostContainer_SetStatusText(DocHost* This, LPCWSTR text)
{
    return E_NOTIMPL;
}

static void WINAPI DocHostContainer_SetURL(DocHost* This, LPCWSTR url)
{

}

static HRESULT DocHostContainer_exec(DocHost *doc_host, const GUID *cmd_group, DWORD cmdid, DWORD execopt, VARIANT *in,
        VARIANT *out)
{
    WebBrowser *This = DOCHOST_THIS(doc_host);
    IOleCommandTarget *cmdtrg = NULL;
    HRESULT hres;

    if(This->client) {
        hres = IOleClientSite_QueryInterface(This->client, &IID_IOleCommandTarget, (void**)&cmdtrg);
        if(FAILED(hres))
            cmdtrg = NULL;
    }

    if(!cmdtrg && This->container) {
        hres = IOleContainer_QueryInterface(This->container, &IID_IOleCommandTarget, (void**)&cmdtrg);
        if(FAILED(hres))
            cmdtrg = NULL;
    }

    if(!cmdtrg)
        return S_OK;

    hres = IOleCommandTarget_Exec(cmdtrg, cmd_group, cmdid, execopt, in, out);
    IOleCommandTarget_Release(cmdtrg);
    if(FAILED(hres))
        FIXME("Exec failed\n");

    return hres;
}

#undef DOCHOST_THIS

static const IDocHostContainerVtbl DocHostContainerVtbl = {
    DocHostContainer_GetDocObjRect,
    DocHostContainer_SetStatusText,
    DocHostContainer_SetURL,
    DocHostContainer_exec
};

static HRESULT WebBrowser_Create(INT version, IUnknown *pOuter, REFIID riid, void **ppv)
{
    WebBrowser *ret;
    HRESULT hres;

    TRACE("(%p %s %p) version=%d\n", pOuter, debugstr_guid(riid), ppv, version);

    ret = heap_alloc_zero(sizeof(WebBrowser));

    ret->lpWebBrowser2Vtbl = &WebBrowser2Vtbl;
    ret->lpServiceProviderVtbl = &ServiceProviderVtbl;
    ret->ref = 1;
    ret->version = version;

    DocHost_Init(&ret->doc_host, (IDispatch*)WEBBROWSER2(ret), &DocHostContainerVtbl);

    ret->visible = VARIANT_TRUE;
    ret->menu_bar = VARIANT_TRUE;
    ret->address_bar = VARIANT_TRUE;
    ret->status_bar = VARIANT_TRUE;
    ret->tool_bar = VARIANT_TRUE;

    WebBrowser_OleObject_Init(ret);
    WebBrowser_ViewObject_Init(ret);
    WebBrowser_DataObject_Init(ret);
    WebBrowser_Persist_Init(ret);
    WebBrowser_ClassInfo_Init(ret);

    HlinkFrame_Init(&ret->hlink_frame, (IUnknown*)WEBBROWSER2(ret), &ret->doc_host);

    SHDOCVW_LockModule();

    hres = IWebBrowser_QueryInterface(WEBBROWSER(ret), riid, ppv);

    IWebBrowser2_Release(WEBBROWSER(ret));
    return hres;
}

HRESULT WebBrowserV1_Create(IUnknown *pOuter, REFIID riid, void **ppv)
{
    return WebBrowser_Create(1, pOuter, riid, ppv);
}

HRESULT WebBrowserV2_Create(IUnknown *pOuter, REFIID riid, void **ppv)
{
    return WebBrowser_Create(2, pOuter, riid, ppv);
}
