/*
 * Copyright 2009 Hans Leidekker for CodeWeavers
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

#include "config.h"
#include <stdarg.h>
#include <stdio.h>

#define COBJMACROS

#include "windef.h"
#include "winbase.h"
#include "winuser.h"
#include "ole2.h"
#include "netfw.h"

#include "wine/debug.h"
#include "wine/unicode.h"
#include "hnetcfg_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(hnetcfg);

typedef struct fw_app
{
    const INetFwAuthorizedApplicationVtbl *vtbl;
    LONG refs;
} fw_app;

static inline fw_app *impl_from_INetFwAuthorizedApplication( INetFwAuthorizedApplication *iface )
{
    return (fw_app *)((char *)iface - FIELD_OFFSET( fw_app, vtbl ));
}

static ULONG WINAPI fw_app_AddRef(
    INetFwAuthorizedApplication *iface )
{
    fw_app *fw_app = impl_from_INetFwAuthorizedApplication( iface );
    return InterlockedIncrement( &fw_app->refs );
}

static ULONG WINAPI fw_app_Release(
    INetFwAuthorizedApplication *iface )
{
    fw_app *fw_app = impl_from_INetFwAuthorizedApplication( iface );
    LONG refs = InterlockedDecrement( &fw_app->refs );
    if (!refs)
    {
        TRACE("destroying %p\n", fw_app);
        HeapFree( GetProcessHeap(), 0, fw_app );
    }
    return refs;
}

static HRESULT WINAPI fw_app_QueryInterface(
    INetFwAuthorizedApplication *iface,
    REFIID riid,
    void **ppvObject )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    TRACE("%p %s %p\n", This, debugstr_guid( riid ), ppvObject );

    if ( IsEqualGUID( riid, &IID_INetFwAuthorizedApplication ) ||
         IsEqualGUID( riid, &IID_IDispatch ) ||
         IsEqualGUID( riid, &IID_IUnknown ) )
    {
        *ppvObject = iface;
    }
    else
    {
        FIXME("interface %s not implemented\n", debugstr_guid(riid));
        return E_NOINTERFACE;
    }
    INetFwAuthorizedApplication_AddRef( iface );
    return S_OK;
}

static HRESULT WINAPI fw_app_GetTypeInfoCount(
    INetFwAuthorizedApplication *iface,
    UINT *pctinfo )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p %p\n", This, pctinfo);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_app_GetTypeInfo(
    INetFwAuthorizedApplication *iface,
    UINT iTInfo,
    LCID lcid,
    ITypeInfo **ppTInfo )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p %u %u %p\n", This, iTInfo, lcid, ppTInfo);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_app_GetIDsOfNames(
    INetFwAuthorizedApplication *iface,
    REFIID riid,
    LPOLESTR *rgszNames,
    UINT cNames,
    LCID lcid,
    DISPID *rgDispId )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p %s %p %u %u %p\n", This, debugstr_guid(riid), rgszNames, cNames, lcid, rgDispId);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_app_Invoke(
    INetFwAuthorizedApplication *iface,
    DISPID dispIdMember,
    REFIID riid,
    LCID lcid,
    WORD wFlags,
    DISPPARAMS *pDispParams,
    VARIANT *pVarResult,
    EXCEPINFO *pExcepInfo,
    UINT *puArgErr )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p %d %s %d %d %p %p %p %p\n", This, dispIdMember, debugstr_guid(riid),
          lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_app_get_Name(
    INetFwAuthorizedApplication *iface,
    BSTR *name )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p, %p\n", This, name);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_app_put_Name(
    INetFwAuthorizedApplication *iface,
    BSTR name )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p, %s\n", This, debugstr_w(name));
    return S_OK;
}

static HRESULT WINAPI fw_app_get_ProcessImageFileName(
    INetFwAuthorizedApplication *iface,
    BSTR *imageFileName )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p, %p\n", This, imageFileName);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_app_put_ProcessImageFileName(
    INetFwAuthorizedApplication *iface,
    BSTR imageFileName )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p, %s\n", This, debugstr_w(imageFileName));
    return S_OK;
}

static HRESULT WINAPI fw_app_get_IpVersion(
    INetFwAuthorizedApplication *iface,
    NET_FW_IP_VERSION *ipVersion )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p, %p\n", This, ipVersion);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_app_put_IpVersion(
    INetFwAuthorizedApplication *iface,
    NET_FW_IP_VERSION ipVersion )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p, %u\n", This, ipVersion);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_app_get_Scope(
    INetFwAuthorizedApplication *iface,
    NET_FW_SCOPE *scope )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p, %p\n", This, scope);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_app_put_Scope(
    INetFwAuthorizedApplication *iface,
    NET_FW_SCOPE scope )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p, %u\n", This, scope);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_app_get_RemoteAddresses(
    INetFwAuthorizedApplication *iface,
    BSTR *remoteAddrs )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p, %p\n", This, remoteAddrs);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_app_put_RemoteAddresses(
    INetFwAuthorizedApplication *iface,
    BSTR remoteAddrs )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p, %s\n", This, debugstr_w(remoteAddrs));
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_app_get_Enabled(
    INetFwAuthorizedApplication *iface,
    VARIANT_BOOL *enabled )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p, %p\n", This, enabled);

    *enabled = VARIANT_FALSE;
    return S_OK;
}

static HRESULT WINAPI fw_app_put_Enabled(
    INetFwAuthorizedApplication *iface,
    VARIANT_BOOL enabled )
{
    fw_app *This = impl_from_INetFwAuthorizedApplication( iface );

    FIXME("%p, %d\n", This, enabled);
    return E_NOTIMPL;
}

static const struct INetFwAuthorizedApplicationVtbl fw_app_vtbl =
{
    fw_app_QueryInterface,
    fw_app_AddRef,
    fw_app_Release,
    fw_app_GetTypeInfoCount,
    fw_app_GetTypeInfo,
    fw_app_GetIDsOfNames,
    fw_app_Invoke,
    fw_app_get_Name,
    fw_app_put_Name,
    fw_app_get_ProcessImageFileName,
    fw_app_put_ProcessImageFileName,
    fw_app_get_IpVersion,
    fw_app_put_IpVersion,
    fw_app_get_Scope,
    fw_app_put_Scope,
    fw_app_get_RemoteAddresses,
    fw_app_put_RemoteAddresses,
    fw_app_get_Enabled,
    fw_app_put_Enabled
};

HRESULT NetFwAuthorizedApplication_create( IUnknown *pUnkOuter, LPVOID *ppObj )
{
    fw_app *fa;

    TRACE("(%p,%p)\n", pUnkOuter, ppObj);

    fa = HeapAlloc( GetProcessHeap(), 0, sizeof(*fa) );
    if (!fa) return E_OUTOFMEMORY;

    fa->vtbl = &fw_app_vtbl;
    fa->refs = 1;

    *ppObj = &fa->vtbl;

    TRACE("returning iface %p\n", *ppObj);
    return S_OK;
}
typedef struct fw_apps
{
    const INetFwAuthorizedApplicationsVtbl *vtbl;
    LONG refs;
} fw_apps;

static inline fw_apps *impl_from_INetFwAuthorizedApplications( INetFwAuthorizedApplications *iface )
{
    return (fw_apps *)((char *)iface - FIELD_OFFSET( fw_apps, vtbl ));
}

static ULONG WINAPI fw_apps_AddRef(
    INetFwAuthorizedApplications *iface )
{
    fw_apps *fw_apps = impl_from_INetFwAuthorizedApplications( iface );
    return InterlockedIncrement( &fw_apps->refs );
}

static ULONG WINAPI fw_apps_Release(
    INetFwAuthorizedApplications *iface )
{
    fw_apps *fw_apps = impl_from_INetFwAuthorizedApplications( iface );
    LONG refs = InterlockedDecrement( &fw_apps->refs );
    if (!refs)
    {
        TRACE("destroying %p\n", fw_apps);
        HeapFree( GetProcessHeap(), 0, fw_apps );
    }
    return refs;
}

static HRESULT WINAPI fw_apps_QueryInterface(
    INetFwAuthorizedApplications *iface,
    REFIID riid,
    void **ppvObject )
{
    fw_apps *This = impl_from_INetFwAuthorizedApplications( iface );

    TRACE("%p %s %p\n", This, debugstr_guid( riid ), ppvObject );

    if ( IsEqualGUID( riid, &IID_INetFwAuthorizedApplications ) ||
         IsEqualGUID( riid, &IID_IDispatch ) ||
         IsEqualGUID( riid, &IID_IUnknown ) )
    {
        *ppvObject = iface;
    }
    else
    {
        FIXME("interface %s not implemented\n", debugstr_guid(riid));
        return E_NOINTERFACE;
    }
    INetFwAuthorizedApplications_AddRef( iface );
    return S_OK;
}

static HRESULT WINAPI fw_apps_GetTypeInfoCount(
    INetFwAuthorizedApplications *iface,
    UINT *pctinfo )
{
    fw_apps *This = impl_from_INetFwAuthorizedApplications( iface );

    FIXME("%p %p\n", This, pctinfo);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_apps_GetTypeInfo(
    INetFwAuthorizedApplications *iface,
    UINT iTInfo,
    LCID lcid,
    ITypeInfo **ppTInfo )
{
    fw_apps *This = impl_from_INetFwAuthorizedApplications( iface );

    FIXME("%p %u %u %p\n", This, iTInfo, lcid, ppTInfo);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_apps_GetIDsOfNames(
    INetFwAuthorizedApplications *iface,
    REFIID riid,
    LPOLESTR *rgszNames,
    UINT cNames,
    LCID lcid,
    DISPID *rgDispId )
{
    fw_apps *This = impl_from_INetFwAuthorizedApplications( iface );

    FIXME("%p %s %p %u %u %p\n", This, debugstr_guid(riid), rgszNames, cNames, lcid, rgDispId);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_apps_Invoke(
    INetFwAuthorizedApplications *iface,
    DISPID dispIdMember,
    REFIID riid,
    LCID lcid,
    WORD wFlags,
    DISPPARAMS *pDispParams,
    VARIANT *pVarResult,
    EXCEPINFO *pExcepInfo,
    UINT *puArgErr )
{
    fw_apps *This = impl_from_INetFwAuthorizedApplications( iface );

    FIXME("%p %d %s %d %d %p %p %p %p\n", This, dispIdMember, debugstr_guid(riid),
          lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_apps_get_Count(
    INetFwAuthorizedApplications *iface,
    LONG *count )
{
    fw_apps *This = impl_from_INetFwAuthorizedApplications( iface );

    FIXME("%p, %p\n", This, count);
    return E_NOTIMPL;
}

static HRESULT WINAPI fw_apps_Add(
    INetFwAuthorizedApplications *iface,
    INetFwAuthorizedApplication *app )
{
    fw_apps *This = impl_from_INetFwAuthorizedApplications( iface );

    FIXME("%p, %p\n", This, app);
    return S_OK;
}

static HRESULT WINAPI fw_apps_Remove(
    INetFwAuthorizedApplications *iface,
    BSTR imageFileName )
{
    fw_apps *This = impl_from_INetFwAuthorizedApplications( iface );

    FIXME("%p, %s\n", This, debugstr_w(imageFileName));
    return S_OK;
}

static HRESULT WINAPI fw_apps_Item(
    INetFwAuthorizedApplications *iface,
    BSTR imageFileName,
    INetFwAuthorizedApplication **app )
{
    fw_apps *This = impl_from_INetFwAuthorizedApplications( iface );

    TRACE("%p, %s, %p\n", This, debugstr_w(imageFileName), app);
    return NetFwAuthorizedApplication_create( NULL, (void **)app );
}

static HRESULT WINAPI fw_apps_get__NewEnum(
    INetFwAuthorizedApplications *iface,
    IUnknown **newEnum )
{
    fw_apps *This = impl_from_INetFwAuthorizedApplications( iface );

    FIXME("%p, %p\n", This, newEnum);
    return E_NOTIMPL;
}

static const struct INetFwAuthorizedApplicationsVtbl fw_apps_vtbl =
{
    fw_apps_QueryInterface,
    fw_apps_AddRef,
    fw_apps_Release,
    fw_apps_GetTypeInfoCount,
    fw_apps_GetTypeInfo,
    fw_apps_GetIDsOfNames,
    fw_apps_Invoke,
    fw_apps_get_Count,
    fw_apps_Add,
    fw_apps_Remove,
    fw_apps_Item,
    fw_apps_get__NewEnum
};

HRESULT NetFwAuthorizedApplications_create( IUnknown *pUnkOuter, LPVOID *ppObj )
{
    fw_apps *fa;

    TRACE("(%p,%p)\n", pUnkOuter, ppObj);

    fa = HeapAlloc( GetProcessHeap(), 0, sizeof(*fa) );
    if (!fa) return E_OUTOFMEMORY;

    fa->vtbl = &fw_apps_vtbl;
    fa->refs = 1;

    *ppObj = &fa->vtbl;

    TRACE("returning iface %p\n", *ppObj);
    return S_OK;
}
