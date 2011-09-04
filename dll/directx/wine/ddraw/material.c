/* Direct3D Material
 * Copyright (c) 2002 Lionel ULMER
 * Copyright (c) 2006 Stefan DÖSINGER
 *
 * This file contains the implementation of Direct3DMaterial.
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
#include "wine/port.h"

#include "ddraw_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(ddraw);

static void dump_material(const D3DMATERIAL *mat)
{
    TRACE("  dwSize : %d\n", mat->dwSize);
}

static inline IDirect3DMaterialImpl *material_from_material1(IDirect3DMaterial *iface)
{
    return (IDirect3DMaterialImpl *)((char*)iface - FIELD_OFFSET(IDirect3DMaterialImpl, IDirect3DMaterial_vtbl));
}

static inline IDirect3DMaterialImpl *material_from_material2(IDirect3DMaterial2 *iface)
{
    return (IDirect3DMaterialImpl *)((char*)iface - FIELD_OFFSET(IDirect3DMaterialImpl, IDirect3DMaterial2_vtbl));
}

/*****************************************************************************
 * IUnknown Methods.
 *****************************************************************************/

/*****************************************************************************
 * IDirect3DMaterial3::QueryInterface
 *
 * QueryInterface for IDirect3DMaterial. Can query all IDirect3DMaterial
 * versions.
 *
 * Params:
 *  riid: Interface id queried for
 *  obj: Address to pass the interface pointer back
 *
 * Returns:
 *  S_OK on success
 *  E_NOINTERFACE if the requested interface wasn't found
 *
 *****************************************************************************/
static HRESULT WINAPI
IDirect3DMaterialImpl_QueryInterface(IDirect3DMaterial3 *iface,
                                     REFIID riid,
                                     LPVOID* obp)
{
    IDirect3DMaterialImpl *This = (IDirect3DMaterialImpl *)iface;

    TRACE("iface %p, riid %s, object %p.\n", iface, debugstr_guid(riid), obp);

    *obp = NULL;

    if ( IsEqualGUID( &IID_IUnknown,  riid ) ) {
        IUnknown_AddRef(iface);
        *obp = iface;
        TRACE("  Creating IUnknown interface at %p.\n", *obp);
        return S_OK;
    }
    if ( IsEqualGUID( &IID_IDirect3DMaterial, riid ) ) {
        IDirect3DMaterial_AddRef((IDirect3DMaterial *)&This->IDirect3DMaterial_vtbl);
        *obp = &This->IDirect3DMaterial_vtbl;
        TRACE("  Creating IDirect3DMaterial interface %p\n", *obp);
        return S_OK;
    }
    if ( IsEqualGUID( &IID_IDirect3DMaterial2, riid ) ) {
        IDirect3DMaterial_AddRef((IDirect3DMaterial2 *)&This->IDirect3DMaterial2_vtbl);
        *obp = &This->IDirect3DMaterial2_vtbl;
        TRACE("  Creating IDirect3DMaterial2 interface %p\n", *obp);
        return S_OK;
    }
    if ( IsEqualGUID( &IID_IDirect3DMaterial3, riid ) ) {
        IDirect3DMaterial3_AddRef((IDirect3DMaterial3 *)This);
        *obp = This;
        TRACE("  Creating IDirect3DMaterial3 interface %p\n", *obp);
        return S_OK;
    }
    FIXME("(%p): interface for IID %s NOT found!\n", This, debugstr_guid(riid));
    return E_NOINTERFACE;
}

/*****************************************************************************
 * IDirect3DMaterial3::AddRef
 *
 * Increases the refcount.
 *
 * Returns:
 *  The new refcount
 *
 *****************************************************************************/
static ULONG WINAPI
IDirect3DMaterialImpl_AddRef(IDirect3DMaterial3 *iface)
{
    IDirect3DMaterialImpl *This = (IDirect3DMaterialImpl *)iface;
    ULONG ref = InterlockedIncrement(&This->ref);

    TRACE("%p increasing refcount to %u.\n", This, ref);

    return ref;
}

/*****************************************************************************
 * IDirect3DMaterial3::Release
 *
 * Reduces the refcount by one. If the refcount falls to 0, the object
 * is destroyed
 *
 * Returns:
 *  The new refcount
 *
 *****************************************************************************/
static ULONG WINAPI
IDirect3DMaterialImpl_Release(IDirect3DMaterial3 *iface)
{
    IDirect3DMaterialImpl *This = (IDirect3DMaterialImpl *)iface;
    ULONG ref = InterlockedDecrement(&This->ref);

    TRACE("%p decreasing refcount to %u.\n", This, ref);

    if (!ref)
    {
        if(This->Handle)
        {
            EnterCriticalSection(&ddraw_cs);
            ddraw_free_handle(&This->ddraw->d3ddevice->handle_table, This->Handle - 1, DDRAW_HANDLE_MATERIAL);
            LeaveCriticalSection(&ddraw_cs);
        }

        HeapFree(GetProcessHeap(), 0, This);
        return 0;
    }
    return ref;
}

/*****************************************************************************
 * IDirect3DMaterial Methods
 *****************************************************************************/

/*****************************************************************************
 * IDirect3DMaterial::Initialize
 *
 * A no-op initialization
 *
 * Params:
 *  Direct3D: Pointer to a Direct3D interface
 *
 * Returns:
 *  D3D_OK
 *
 *****************************************************************************/
static HRESULT WINAPI
IDirect3DMaterialImpl_Initialize(IDirect3DMaterial *iface,
                                  IDirect3D *Direct3D)
{
    TRACE("iface %p, d3d %p.\n", iface, Direct3D);

    return D3D_OK;
}

/*****************************************************************************
 * IDirect3DMaterial::Reserve
 *
 * DirectX 5 sdk: "The IDirect3DMaterial2::Reserve method is not implemented"
 * Odd. They seem to have mixed their interfaces.
 *
 * Returns:
 *  DDERR_UNSUPPORTED
 *
 *****************************************************************************/
static HRESULT WINAPI
IDirect3DMaterialImpl_Reserve(IDirect3DMaterial *iface)
{
    TRACE("iface %p.\n", iface);

    return DDERR_UNSUPPORTED;
}

/*****************************************************************************
 * IDirect3DMaterial::Unreserve
 *
 * Not supported too
 *
 * Returns:
 *  DDERR_UNSUPPORTED
 *
 *****************************************************************************/
static HRESULT WINAPI
IDirect3DMaterialImpl_Unreserve(IDirect3DMaterial *iface)
{
    TRACE("iface %p.\n", iface);

    return DDERR_UNSUPPORTED;
}

/*****************************************************************************
 * IDirect3DMaterial3::SetMaterial
 *
 * Sets the material description
 *
 * Params:
 *  Mat: Material to set
 *
 * Returns:
 *  D3D_OK on success
 *  DDERR_INVALIDPARAMS if Mat is NULL
 *
 *****************************************************************************/
static HRESULT WINAPI
IDirect3DMaterialImpl_SetMaterial(IDirect3DMaterial3 *iface,
                                  D3DMATERIAL *lpMat)
{
    IDirect3DMaterialImpl *This = (IDirect3DMaterialImpl *)iface;

    TRACE("iface %p, material %p.\n", iface, lpMat);
    if (TRACE_ON(ddraw))
        dump_material(lpMat);

    /* Stores the material */
    EnterCriticalSection(&ddraw_cs);
    memset(&This->mat, 0, sizeof(This->mat));
    memcpy(&This->mat, lpMat, lpMat->dwSize);
    LeaveCriticalSection(&ddraw_cs);

    return DD_OK;
}

/*****************************************************************************
 * IDirect3DMaterial3::GetMaterial
 *
 * Returns the material assigned to this interface
 *
 * Params:
 *  Mat: Pointer to a D3DMATERIAL structure to store the material description
 *
 * Returns:
 *  D3D_OK on success
 *  DDERR_INVALIDPARAMS if Mat is NULL
 *
 *****************************************************************************/
static HRESULT WINAPI
IDirect3DMaterialImpl_GetMaterial(IDirect3DMaterial3 *iface,
                                  D3DMATERIAL *lpMat)
{
    IDirect3DMaterialImpl *This = (IDirect3DMaterialImpl *)iface;
    DWORD dwSize;

    TRACE("iface %p, material %p.\n", iface, lpMat);
    if (TRACE_ON(ddraw))
    {
        TRACE("  Returning material : ");
        dump_material(&This->mat);
    }

    /* Copies the material structure */
    EnterCriticalSection(&ddraw_cs);
    dwSize = lpMat->dwSize;
    memcpy(lpMat, &This->mat, dwSize);
    LeaveCriticalSection(&ddraw_cs);

    return DD_OK;
}

/*****************************************************************************
 * IDirect3DMaterial3::GetHandle
 *
 * Returns a handle for the material interface. The handle is simply a
 * pointer to the material implementation
 *
 * Params:
 *  Direct3DDevice3: The device this handle is assigned to
 *  Handle: Address to write the handle to
 *
 * Returns:
 *  D3D_OK on success
 *  DDERR_INVALIDPARAMS if Handle is NULL
 *
 *****************************************************************************/
static HRESULT WINAPI
IDirect3DMaterialImpl_GetHandle(IDirect3DMaterial3 *iface,
                                IDirect3DDevice3 *lpDirect3DDevice3,
                                D3DMATERIALHANDLE *lpHandle)
{
    IDirect3DMaterialImpl *This = (IDirect3DMaterialImpl *)iface;
    IDirect3DDeviceImpl *device = device_from_device3(lpDirect3DDevice3);

    TRACE("iface %p, device %p, handle %p.\n", iface, lpDirect3DDevice3, lpHandle);

    EnterCriticalSection(&ddraw_cs);
    This->active_device = device;
    if(!This->Handle)
    {
        DWORD h = ddraw_allocate_handle(&device->handle_table, This, DDRAW_HANDLE_MATERIAL);
        if (h == DDRAW_INVALID_HANDLE)
        {
            ERR("Failed to allocate a material handle.\n");
            LeaveCriticalSection(&ddraw_cs);
            return DDERR_INVALIDPARAMS;   /* Unchecked */
        }

        This->Handle = h + 1;
    }
    *lpHandle = This->Handle;
    TRACE(" returning handle %08x.\n", *lpHandle);
    LeaveCriticalSection(&ddraw_cs);

    return D3D_OK;
}

static HRESULT WINAPI IDirect3DMaterialImpl_2_GetHandle(IDirect3DMaterial2 *iface,
        IDirect3DDevice2 *lpDirect3DDevice2, D3DMATERIALHANDLE *lpHandle)
{
    TRACE("iface %p, device %p, handle %p.\n", iface, lpDirect3DDevice2, lpHandle);

    return IDirect3DMaterial3_GetHandle((IDirect3DMaterial3 *)material_from_material2(iface), lpDirect3DDevice2 ?
            (IDirect3DDevice3 *)&device_from_device2(lpDirect3DDevice2)->IDirect3DDevice3_vtbl : NULL, lpHandle);
}

static HRESULT WINAPI IDirect3DMaterialImpl_1_GetHandle(IDirect3DMaterial *iface,
        IDirect3DDevice *lpDirect3DDevice, D3DMATERIALHANDLE *lpHandle)
{
    TRACE("iface %p, device %p, handle %p.\n", iface, lpDirect3DDevice, lpHandle);

    return IDirect3DMaterial3_GetHandle((IDirect3DMaterial3 *)material_from_material1(iface), lpDirect3DDevice ?
            (IDirect3DDevice3 *)&device_from_device1(lpDirect3DDevice)->IDirect3DDevice3_vtbl : NULL, lpHandle);
}

static HRESULT WINAPI IDirect3DMaterialImpl_2_QueryInterface(LPDIRECT3DMATERIAL2 iface, REFIID riid,
        LPVOID* obp)
{
    TRACE("iface %p, riid %s, object %p.\n", iface, debugstr_guid(riid), obp);

    return IDirect3DMaterial3_QueryInterface((IDirect3DMaterial3 *)material_from_material2(iface), riid, obp);
}

static HRESULT WINAPI IDirect3DMaterialImpl_1_QueryInterface(LPDIRECT3DMATERIAL iface, REFIID riid,
        LPVOID* obp)
{
    TRACE("iface %p, riid %s, object %p.\n", iface, debugstr_guid(riid), obp);

    return IDirect3DMaterial3_QueryInterface((IDirect3DMaterial3 *)material_from_material1(iface), riid, obp);
}

static ULONG WINAPI IDirect3DMaterialImpl_2_AddRef(LPDIRECT3DMATERIAL2 iface)
{
    TRACE("iface %p.\n", iface);

    return IDirect3DMaterial3_AddRef((IDirect3DMaterial3 *)material_from_material2(iface));
}

static ULONG WINAPI IDirect3DMaterialImpl_1_AddRef(LPDIRECT3DMATERIAL iface)
{
    TRACE("iface %p.\n", iface);

    return IDirect3DMaterial3_AddRef((IDirect3DMaterial3 *)material_from_material1(iface));
}

static ULONG WINAPI IDirect3DMaterialImpl_2_Release(LPDIRECT3DMATERIAL2 iface)
{
    TRACE("iface %p.\n", iface);

    return IDirect3DMaterial3_Release((IDirect3DMaterial3 *)material_from_material2(iface));
}

static ULONG WINAPI IDirect3DMaterialImpl_1_Release(LPDIRECT3DMATERIAL iface)
{
    TRACE("iface %p.\n", iface);

    return IDirect3DMaterial3_Release((IDirect3DMaterial3 *)material_from_material1(iface));
}

static HRESULT WINAPI IDirect3DMaterialImpl_2_SetMaterial(LPDIRECT3DMATERIAL2 iface,
        LPD3DMATERIAL lpMat)
{
    TRACE("iface %p, material %p.\n", iface, lpMat);

    return IDirect3DMaterial3_SetMaterial((IDirect3DMaterial3 *)material_from_material2(iface), lpMat);
}

static HRESULT WINAPI IDirect3DMaterialImpl_1_SetMaterial(LPDIRECT3DMATERIAL iface,
        LPD3DMATERIAL lpMat)
{
    TRACE("iface %p, material %p.\n", iface, lpMat);

    return IDirect3DMaterial3_SetMaterial((IDirect3DMaterial3 *)material_from_material1(iface), lpMat);
}

static HRESULT WINAPI IDirect3DMaterialImpl_2_GetMaterial(LPDIRECT3DMATERIAL2 iface,
        LPD3DMATERIAL lpMat)
{
    TRACE("iface %p, material %p.\n", iface, lpMat);

    return IDirect3DMaterial3_GetMaterial((IDirect3DMaterial3 *)material_from_material2(iface), lpMat);
}

static HRESULT WINAPI IDirect3DMaterialImpl_1_GetMaterial(LPDIRECT3DMATERIAL iface,
        LPD3DMATERIAL lpMat)
{
    TRACE("iface %p, material %p.\n", iface, lpMat);

    return IDirect3DMaterial3_GetMaterial((IDirect3DMaterial3 *)material_from_material1(iface), lpMat);
}


/*****************************************************************************
 * material_activate
 *
 * Uses IDirect3DDevice7::SetMaterial to activate the material
 *
 * Params:
 *  This: Pointer to the material implementation to activate
 *
 *****************************************************************************/
void material_activate(IDirect3DMaterialImpl* This)
{
    D3DMATERIAL7 d3d7mat;

    TRACE("Activating material %p\n", This);
    d3d7mat.u.diffuse = This->mat.u.diffuse;
    d3d7mat.u1.ambient = This->mat.u1.ambient;
    d3d7mat.u2.specular = This->mat.u2.specular;
    d3d7mat.u3.emissive = This->mat.u3.emissive;
    d3d7mat.u4.power = This->mat.u4.power;

    IDirect3DDevice7_SetMaterial((IDirect3DDevice7 *)This->active_device, &d3d7mat);
}

static const struct IDirect3DMaterial3Vtbl d3d_material3_vtbl =
{
    /*** IUnknown Methods ***/
    IDirect3DMaterialImpl_QueryInterface,
    IDirect3DMaterialImpl_AddRef,
    IDirect3DMaterialImpl_Release,
    /*** IDirect3DMaterial3 Methods ***/
    IDirect3DMaterialImpl_SetMaterial,
    IDirect3DMaterialImpl_GetMaterial,
    IDirect3DMaterialImpl_GetHandle,
};

static const struct IDirect3DMaterial2Vtbl d3d_material2_vtbl =
{
    /*** IUnknown Methods ***/
    IDirect3DMaterialImpl_2_QueryInterface,
    IDirect3DMaterialImpl_2_AddRef,
    IDirect3DMaterialImpl_2_Release,
    /*** IDirect3DMaterial2 Methods ***/
    IDirect3DMaterialImpl_2_SetMaterial,
    IDirect3DMaterialImpl_2_GetMaterial,
    IDirect3DMaterialImpl_2_GetHandle,
};

static const struct IDirect3DMaterialVtbl d3d_material1_vtbl =
{
    /*** IUnknown Methods ***/
    IDirect3DMaterialImpl_1_QueryInterface,
    IDirect3DMaterialImpl_1_AddRef,
    IDirect3DMaterialImpl_1_Release,
    /*** IDirect3DMaterial1 Methods ***/
    IDirect3DMaterialImpl_Initialize,
    IDirect3DMaterialImpl_1_SetMaterial,
    IDirect3DMaterialImpl_1_GetMaterial,
    IDirect3DMaterialImpl_1_GetHandle,
    IDirect3DMaterialImpl_Reserve,
    IDirect3DMaterialImpl_Unreserve
};

void d3d_material_init(IDirect3DMaterialImpl *material, IDirectDrawImpl *ddraw)
{
    material->lpVtbl = &d3d_material3_vtbl;
    material->IDirect3DMaterial2_vtbl = &d3d_material2_vtbl;
    material->IDirect3DMaterial_vtbl = &d3d_material1_vtbl;
    material->ref = 1;
    material->ddraw = ddraw;
}
