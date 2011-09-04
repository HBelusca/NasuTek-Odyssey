/*
 * RichEdit - functions and interfaces around CreateTextServices
 *
 * Copyright 2005, 2006, Maarten Lankhorst
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

#define NONAMELESSSTRUCT
#define NONAMELESSUNION
#define COBJMACROS

#include "editor.h"
#include "ole2.h"
#include "oleauto.h"
#include "richole.h"
#include "imm.h"
#include "textserv.h"
#include "wine/debug.h"
#include "editstr.h"

#ifdef __i386__  /* thiscall functions are i386-specific */

#define THISCALL(func) __thiscall_ ## func
#define DEFINE_THISCALL_WRAPPER(func,args) \
   extern typeof(func) THISCALL(func); \
   __ASM_STDCALL_FUNC(__thiscall_ ## func, args, \
                   "popl %eax\n\t" \
                   "pushl %ecx\n\t" \
                   "pushl %eax\n\t" \
                   "jmp " __ASM_NAME(#func) __ASM_STDCALL(args) )
#else /* __i386__ */

#define THISCALL(func) func
#define DEFINE_THISCALL_WRAPPER(func,args) /* nothing */

#endif /* __i386__ */

WINE_DEFAULT_DEBUG_CHANNEL(richedit);

typedef struct ITextServicesImpl {
   const ITextServicesVtbl *lpVtbl;
   ITextHost *pMyHost;
   LONG ref;
   CRITICAL_SECTION csTxtSrv;
   ME_TextEditor *editor;
   char spare[256];
} ITextServicesImpl;

static const ITextServicesVtbl textservices_Vtbl;

/******************************************************************
 *        CreateTextServices (RICHED20.4)
 */
HRESULT WINAPI CreateTextServices(IUnknown  * pUnkOuter,
                                  ITextHost * pITextHost,
                                  IUnknown  **ppUnk)
{
   ITextServicesImpl *ITextImpl;
   HRESULT hres;
   TRACE("%p %p --> %p\n", pUnkOuter, pITextHost, ppUnk);
   if (pITextHost == NULL)
      return E_POINTER;

   ITextImpl = CoTaskMemAlloc(sizeof(*ITextImpl));
   if (ITextImpl == NULL)
      return E_OUTOFMEMORY;
   InitializeCriticalSection(&ITextImpl->csTxtSrv);
   ITextImpl->csTxtSrv.DebugInfo->Spare[0] = (DWORD_PTR)(__FILE__ ": ITextServicesImpl.csTxtSrv");
   ITextImpl->ref = 1;
   ITextHost_AddRef(pITextHost);
   ITextImpl->pMyHost = pITextHost;
   ITextImpl->lpVtbl = &textservices_Vtbl;
   ITextImpl->editor = ME_MakeEditor(pITextHost, FALSE);
   ITextImpl->editor->exStyleFlags = 0;
   ITextImpl->editor->rcFormat.left = 0;
   ITextImpl->editor->rcFormat.top = 0;
   ITextImpl->editor->rcFormat.right = 0;
   ITextImpl->editor->rcFormat.bottom = 0;
   ME_HandleMessage(ITextImpl->editor, WM_CREATE, 0, 0, TRUE, &hres);

   if (pUnkOuter)
   {
      FIXME("Support aggregation\n");
      return CLASS_E_NOAGGREGATION;
   }

   *ppUnk = (IUnknown *)ITextImpl;
   return S_OK;
}

#define ICOM_THIS_MULTI(impl,field,iface) \
	            impl* const This=(impl*)((char*)(iface) - offsetof(impl,field))

static HRESULT WINAPI fnTextSrv_QueryInterface(ITextServices * iface,
                                               REFIID riid,
                                               LPVOID * ppv)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);
   TRACE("(%p/%p)->(%s, %p)\n", This, iface, debugstr_guid(riid), ppv);
   *ppv = NULL;
   if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_ITextServices))
      *ppv = This;

   if (*ppv)
   {
      IUnknown_AddRef((IUnknown *)(*ppv));
      TRACE ("-- Interface = %p\n", *ppv);
      return S_OK;
   }
   FIXME("Unknown interface: %s\n", debugstr_guid(riid));
   return E_NOINTERFACE;
}

static ULONG WINAPI fnTextSrv_AddRef(ITextServices *iface)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);
   DWORD ref = InterlockedIncrement(&This->ref);

   TRACE("(%p/%p)->() AddRef from %d\n", This, iface, ref - 1);
   return ref;
}

static ULONG WINAPI fnTextSrv_Release(ITextServices *iface)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);
   DWORD ref = InterlockedDecrement(&This->ref);

   TRACE("(%p/%p)->() Release from %d\n", This, iface, ref + 1);

   if (!ref)
   {
      ITextHost_Release(This->pMyHost);
      This->csTxtSrv.DebugInfo->Spare[0] = 0;
      DeleteCriticalSection(&This->csTxtSrv);
      CoTaskMemFree(This);
   }
   return ref;
}

HRESULT WINAPI fnTextSrv_TxSendMessage(ITextServices *iface,
                                       UINT msg,
                                       WPARAM wparam,
                                       LPARAM lparam,
                                       LRESULT* plresult)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);
   HRESULT hresult;
   LRESULT lresult;

   lresult = ME_HandleMessage(This->editor, msg, wparam, lparam, TRUE, &hresult);
   if (plresult) *plresult = lresult;
   return hresult;
}

HRESULT WINAPI fnTextSrv_TxDraw(ITextServices *iface,
                                DWORD dwDrawAspect,
                                LONG lindex,
                                void* pvAspect,
                                DVTARGETDEVICE* ptd,
                                HDC hdcDraw,
                                HDC hdcTargetDev,
                                LPCRECTL lprcBounds,
                                LPCRECTL lprcWBounds,
                                LPRECT lprcUpdate,
                                BOOL (CALLBACK * pfnContinue)(DWORD),
                                DWORD dwContinue,
                                LONG lViewId)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   FIXME("%p: STUB\n", This);
   return E_NOTIMPL;
}

HRESULT WINAPI fnTextSrv_TxGetHScroll(ITextServices *iface,
                                      LONG* plMin,
                                      LONG* plMax,
                                      LONG* plPos,
                                      LONG* plPage,
                                      BOOL* pfEnabled)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   *plMin = This->editor->horz_si.nMin;
   *plMax = This->editor->horz_si.nMax;
   *plPos = This->editor->horz_si.nPos;
   *plPage = This->editor->horz_si.nPage;
   *pfEnabled = (This->editor->styleFlags & WS_HSCROLL) != 0;
   return S_OK;
}

HRESULT WINAPI fnTextSrv_TxGetVScroll(ITextServices *iface,
                                      LONG* plMin,
                                      LONG* plMax,
                                      LONG* plPos,
                                      LONG* plPage,
                                      BOOL* pfEnabled)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   *plMin = This->editor->vert_si.nMin;
   *plMax = This->editor->vert_si.nMax;
   *plPos = This->editor->vert_si.nPos;
   *plPage = This->editor->vert_si.nPage;
   *pfEnabled = (This->editor->styleFlags & WS_VSCROLL) != 0;
   return S_OK;
}

HRESULT WINAPI fnTextSrv_OnTxSetCursor(ITextServices *iface,
                                       DWORD dwDrawAspect,
                                       LONG lindex,
                                       void* pvAspect,
                                       DVTARGETDEVICE* ptd,
                                       HDC hdcDraw,
                                       HDC hicTargetDev,
                                       LPCRECT lprcClient,
                                       INT x, INT y)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   FIXME("%p: STUB\n", This);
   return E_NOTIMPL;
}

HRESULT WINAPI fnTextSrv_TxQueryHitPoint(ITextServices *iface,
                                         DWORD dwDrawAspect,
                                         LONG lindex,
                                         void* pvAspect,
                                         DVTARGETDEVICE* ptd,
                                         HDC hdcDraw,
                                         HDC hicTargetDev,
                                         LPCRECT lprcClient,
                                         INT x, INT y,
                                         DWORD* pHitResult)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   FIXME("%p: STUB\n", This);
   return E_NOTIMPL;
}

HRESULT WINAPI fnTextSrv_OnTxInplaceActivate(ITextServices *iface,
                                             LPCRECT prcClient)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   FIXME("%p: STUB\n", This);
   return E_NOTIMPL;
}

HRESULT WINAPI fnTextSrv_OnTxInplaceDeactivate(ITextServices *iface)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   FIXME("%p: STUB\n", This);
   return E_NOTIMPL;
}

HRESULT WINAPI fnTextSrv_OnTxUIActivate(ITextServices *iface)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   FIXME("%p: STUB\n", This);
   return E_NOTIMPL;
}

HRESULT WINAPI fnTextSrv_OnTxUIDeactivate(ITextServices *iface)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   FIXME("%p: STUB\n", This);
   return E_NOTIMPL;
}

HRESULT WINAPI fnTextSrv_TxGetText(ITextServices *iface,
                                   BSTR* pbstrText)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);
   int length;

   length = ME_GetTextLength(This->editor);
   if (length)
   {
      ME_Cursor start;
      BSTR bstr;
      bstr = SysAllocStringByteLen(NULL, length * sizeof(WCHAR));
      if (bstr == NULL)
         return E_OUTOFMEMORY;

      ME_CursorFromCharOfs(This->editor, 0, &start);
      ME_GetTextW(This->editor, bstr, length, &start, INT_MAX, FALSE);
      *pbstrText = bstr;
   } else {
      *pbstrText = NULL;
   }

   return S_OK;
}

HRESULT WINAPI fnTextSrv_TxSetText(ITextServices *iface,
                                   LPCWSTR pszText)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);
   ME_Cursor cursor;

   ME_SetCursorToStart(This->editor, &cursor);
   ME_InternalDeleteText(This->editor, &cursor,
                         ME_GetTextLength(This->editor), FALSE);
   ME_InsertTextFromCursor(This->editor, 0, pszText, -1,
                           This->editor->pBuffer->pDefaultStyle);
   ME_SetSelection(This->editor, 0, 0);
   This->editor->nModifyStep = 0;
   OleFlushClipboard();
   ME_EmptyUndoStack(This->editor);
   ME_UpdateRepaint(This->editor);

   return S_OK;
}

HRESULT WINAPI fnTextSrv_TxGetCurrentTargetX(ITextServices *iface,
                                             LONG* x)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   FIXME("%p: STUB\n", This);
   return E_NOTIMPL;
}

HRESULT WINAPI fnTextSrv_TxGetBaseLinePos(ITextServices *iface,
                                          LONG* x)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   FIXME("%p: STUB\n", This);
   return E_NOTIMPL;
}

HRESULT WINAPI fnTextSrv_TxGetNaturalSize(ITextServices *iface,
                                          DWORD dwAspect,
                                          HDC hdcDraw,
                                          HDC hicTargetDev,
                                          DVTARGETDEVICE* ptd,
                                          DWORD dwMode,
                                          const SIZEL* psizelExtent,
                                          LONG* pwidth,
                                          LONG* pheight)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   FIXME("%p: STUB\n", This);
   return E_NOTIMPL;
}

HRESULT WINAPI fnTextSrv_TxGetDropTarget(ITextServices *iface,
                                         IDropTarget** ppDropTarget)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   FIXME("%p: STUB\n", This);
   return E_NOTIMPL;
}

HRESULT WINAPI fnTextSrv_OnTxPropertyBitsChange(ITextServices *iface,
                                                DWORD dwMask,
                                                DWORD dwBits)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   FIXME("%p: STUB\n", This);
   return E_NOTIMPL;
}

HRESULT WINAPI fnTextSrv_TxGetCachedSize(ITextServices *iface,
                                         DWORD* pdwWidth,
                                         DWORD* pdwHeight)
{
   ICOM_THIS_MULTI(ITextServicesImpl, lpVtbl, iface);

   FIXME("%p: STUB\n", This);
   return E_NOTIMPL;
}

DEFINE_THISCALL_WRAPPER(fnTextSrv_TxSendMessage,20)
DEFINE_THISCALL_WRAPPER(fnTextSrv_TxDraw,52)
DEFINE_THISCALL_WRAPPER(fnTextSrv_TxGetHScroll,24)
DEFINE_THISCALL_WRAPPER(fnTextSrv_TxGetVScroll,24)
DEFINE_THISCALL_WRAPPER(fnTextSrv_OnTxSetCursor,40)
DEFINE_THISCALL_WRAPPER(fnTextSrv_TxQueryHitPoint,44)
DEFINE_THISCALL_WRAPPER(fnTextSrv_OnTxInplaceActivate,8)
DEFINE_THISCALL_WRAPPER(fnTextSrv_OnTxInplaceDeactivate,4)
DEFINE_THISCALL_WRAPPER(fnTextSrv_OnTxUIActivate,4)
DEFINE_THISCALL_WRAPPER(fnTextSrv_OnTxUIDeactivate,4)
DEFINE_THISCALL_WRAPPER(fnTextSrv_TxGetText,8)
DEFINE_THISCALL_WRAPPER(fnTextSrv_TxSetText,8)
DEFINE_THISCALL_WRAPPER(fnTextSrv_TxGetCurrentTargetX,8)
DEFINE_THISCALL_WRAPPER(fnTextSrv_TxGetBaseLinePos,8)
DEFINE_THISCALL_WRAPPER(fnTextSrv_TxGetNaturalSize,36)
DEFINE_THISCALL_WRAPPER(fnTextSrv_TxGetDropTarget,8)
DEFINE_THISCALL_WRAPPER(fnTextSrv_OnTxPropertyBitsChange,12)
DEFINE_THISCALL_WRAPPER(fnTextSrv_TxGetCachedSize,12)

static const ITextServicesVtbl textservices_Vtbl =
{
   fnTextSrv_QueryInterface,
   fnTextSrv_AddRef,
   fnTextSrv_Release,
   THISCALL(fnTextSrv_TxSendMessage),
   THISCALL(fnTextSrv_TxDraw),
   THISCALL(fnTextSrv_TxGetHScroll),
   THISCALL(fnTextSrv_TxGetVScroll),
   THISCALL(fnTextSrv_OnTxSetCursor),
   THISCALL(fnTextSrv_TxQueryHitPoint),
   THISCALL(fnTextSrv_OnTxInplaceActivate),
   THISCALL(fnTextSrv_OnTxInplaceDeactivate),
   THISCALL(fnTextSrv_OnTxUIActivate),
   THISCALL(fnTextSrv_OnTxUIDeactivate),
   THISCALL(fnTextSrv_TxGetText),
   THISCALL(fnTextSrv_TxSetText),
   THISCALL(fnTextSrv_TxGetCurrentTargetX),
   THISCALL(fnTextSrv_TxGetBaseLinePos),
   THISCALL(fnTextSrv_TxGetNaturalSize),
   THISCALL(fnTextSrv_TxGetDropTarget),
   THISCALL(fnTextSrv_OnTxPropertyBitsChange),
   THISCALL(fnTextSrv_TxGetCachedSize)
};
