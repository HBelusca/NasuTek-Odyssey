/*
 *	                      ItemMonikers implementation
 *
 *           Copyright 1999  Noomen Hamza
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

#include <assert.h>
#include <stdarg.h>
#include <string.h>

#define COBJMACROS
#define NONAMELESSUNION
#define NONAMELESSSTRUCT

#include "winerror.h"
#include "windef.h"
#include "winbase.h"
#include "winuser.h"
#include "winnls.h"
#include "wine/debug.h"
#include "ole2.h"
#include "wine/unicode.h"
#include "moniker.h"

WINE_DEFAULT_DEBUG_CHANNEL(ole);

/* ItemMoniker data structure */
typedef struct ItemMonikerImpl{

    const IMonikerVtbl*  lpvtbl1;  /* VTable relative to the IMoniker interface.*/

    /* The ROT (RunningObjectTable implementation) uses the IROTData interface to test whether
     * two monikers are equal. That's whay IROTData interface is implemented by monikers.
     */
    const IROTDataVtbl*  lpvtbl2;  /* VTable relative to the IROTData interface.*/

    LONG ref; /* reference counter for this object */

    LPOLESTR itemName; /* item name identified by this ItemMoniker */

    LPOLESTR itemDelimiter; /* Delimiter string */

    IUnknown *pMarshal; /* custom marshaler */
} ItemMonikerImpl;

static inline IMoniker *impl_from_IROTData( IROTData *iface )
{
    return (IMoniker *)((char*)iface - FIELD_OFFSET(ItemMonikerImpl, lpvtbl2));
}

static HRESULT ItemMonikerImpl_Destroy(ItemMonikerImpl* iface);

/*******************************************************************************
 *        ItemMoniker_QueryInterface
 *******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_QueryInterface(IMoniker* iface,REFIID riid,void** ppvObject)
{
    ItemMonikerImpl *This = (ItemMonikerImpl *)iface;

  TRACE("(%p,%s,%p)\n",This,debugstr_guid(riid),ppvObject);

  /* Perform a sanity check on the parameters.*/
    if ( (This==0) || (ppvObject==0) )
	return E_INVALIDARG;

  /* Initialize the return parameter */
  *ppvObject = 0;

  /* Compare the riid with the interface IDs implemented by this object.*/
  if (IsEqualIID(&IID_IUnknown, riid) ||
      IsEqualIID(&IID_IPersist, riid) ||
      IsEqualIID(&IID_IPersistStream, riid) ||
      IsEqualIID(&IID_IMoniker, riid)
     )
      *ppvObject = iface;

    else if (IsEqualIID(&IID_IROTData, riid))
        *ppvObject = &This->lpvtbl2;
    else if (IsEqualIID(&IID_IMarshal, riid))
    {
        HRESULT hr = S_OK;
        if (!This->pMarshal)
            hr = MonikerMarshal_Create(iface, &This->pMarshal);
        if (hr != S_OK)
            return hr;
        return IUnknown_QueryInterface(This->pMarshal, riid, ppvObject);
    }

  /* Check that we obtained an interface.*/
    if ((*ppvObject)==0)
        return E_NOINTERFACE;

   /* Query Interface always increases the reference count by one when it is successful */
  IMoniker_AddRef(iface);

  return S_OK;
}

/******************************************************************************
 *        ItemMoniker_AddRef
 ******************************************************************************/
static ULONG WINAPI ItemMonikerImpl_AddRef(IMoniker* iface)
{
    ItemMonikerImpl *This = (ItemMonikerImpl *)iface;

    TRACE("(%p)\n",This);

    return InterlockedIncrement(&This->ref);
}

/******************************************************************************
 *        ItemMoniker_Release
 ******************************************************************************/
static ULONG WINAPI ItemMonikerImpl_Release(IMoniker* iface)
{
    ItemMonikerImpl *This = (ItemMonikerImpl *)iface;
    ULONG ref;

    TRACE("(%p)\n",This);

    ref = InterlockedDecrement(&This->ref);

    /* destroy the object if there's no more reference on it */
    if (ref == 0) ItemMonikerImpl_Destroy(This);

    return ref;
}

/******************************************************************************
 *        ItemMoniker_GetClassID
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_GetClassID(IMoniker* iface,CLSID *pClassID)
{
    TRACE("(%p,%p)\n",iface,pClassID);

    if (pClassID==NULL)
        return E_POINTER;

    *pClassID = CLSID_ItemMoniker;

    return S_OK;
}

/******************************************************************************
 *        ItemMoniker_IsDirty
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_IsDirty(IMoniker* iface)
{
    /* Note that the OLE-provided implementations of the IPersistStream::IsDirty
       method in the OLE-provided moniker interfaces always return S_FALSE because
       their internal state never changes. */

    TRACE("(%p)\n",iface);

    return S_FALSE;
}

/******************************************************************************
 *        ItemMoniker_Load
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_Load(IMoniker* iface,IStream* pStm)
{

    ItemMonikerImpl *This = (ItemMonikerImpl *)iface;
    HRESULT res;
    DWORD delimiterLength,nameLength,lenW;
    CHAR *itemNameA,*itemDelimiterA;
    ULONG bread;

    TRACE("\n");

    /* for more details about data read by this function see comments of ItemMonikerImpl_Save function */

    /* read item delimiter string length + 1 */
    res=IStream_Read(pStm,&delimiterLength,sizeof(DWORD),&bread);
    if (bread != sizeof(DWORD))
        return E_FAIL;

    /* read item delimiter string */
    if (!(itemDelimiterA=HeapAlloc(GetProcessHeap(),0,delimiterLength)))
        return E_OUTOFMEMORY;
    res=IStream_Read(pStm,itemDelimiterA,delimiterLength,&bread);
    if (bread != delimiterLength)
    {
        HeapFree( GetProcessHeap(), 0, itemDelimiterA );
        return E_FAIL;
    }

    lenW = MultiByteToWideChar( CP_ACP, 0, itemDelimiterA, -1, NULL, 0 );
    This->itemDelimiter=HeapReAlloc(GetProcessHeap(),0,This->itemDelimiter,lenW*sizeof(WCHAR));
    if (!This->itemDelimiter)
    {
        HeapFree( GetProcessHeap(), 0, itemDelimiterA );
        return E_OUTOFMEMORY;
    }
    MultiByteToWideChar( CP_ACP, 0, itemDelimiterA, -1, This->itemDelimiter, lenW );
    HeapFree( GetProcessHeap(), 0, itemDelimiterA );

    /* read item name string length + 1*/
    res=IStream_Read(pStm,&nameLength,sizeof(DWORD),&bread);
    if (bread != sizeof(DWORD))
        return E_FAIL;

    /* read item name string */
    if (!(itemNameA=HeapAlloc(GetProcessHeap(),0,nameLength)))
        return E_OUTOFMEMORY;
    res=IStream_Read(pStm,itemNameA,nameLength,&bread);
    if (bread != nameLength)
    {
        HeapFree( GetProcessHeap(), 0, itemNameA );
        return E_FAIL;
    }

    lenW = MultiByteToWideChar( CP_ACP, 0, itemNameA, -1, NULL, 0 );
    This->itemName=HeapReAlloc(GetProcessHeap(),0,This->itemName,lenW*sizeof(WCHAR));
    if (!This->itemName)
    {
        HeapFree( GetProcessHeap(), 0, itemNameA );
        return E_OUTOFMEMORY;
    }
    MultiByteToWideChar( CP_ACP, 0, itemNameA, -1, This->itemName, lenW );
    HeapFree( GetProcessHeap(), 0, itemNameA );

    return res;
}

/******************************************************************************
 *        ItemMoniker_Save
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_Save(IMoniker* iface,
                                           IStream* pStm,/* pointer to the stream where the object is to be saved */
                                           BOOL fClearDirty)/* Specifies whether to clear the dirty flag */
{
    ItemMonikerImpl *This = (ItemMonikerImpl *)iface;
    HRESULT res;
    CHAR *itemNameA,*itemDelimiterA;

    /* data written by this function are : 1) DWORD : size of item delimiter string ('\0' included ) */
    /*                                    2) String (type A): item delimiter string ('\0' included)          */
    /*                                    3) DWORD : size of item name string ('\0' included)       */
    /*                                    4) String (type A): item name string ('\0' included)               */

    DWORD nameLength = WideCharToMultiByte( CP_ACP, 0, This->itemName, -1, NULL, 0, NULL, NULL);
    DWORD delimiterLength = WideCharToMultiByte( CP_ACP, 0, This->itemDelimiter, -1, NULL, 0, NULL, NULL);
    itemNameA=HeapAlloc(GetProcessHeap(),0,nameLength);
    itemDelimiterA=HeapAlloc(GetProcessHeap(),0,delimiterLength);
    WideCharToMultiByte( CP_ACP, 0, This->itemName, -1, itemNameA, nameLength, NULL, NULL);
    WideCharToMultiByte( CP_ACP, 0, This->itemDelimiter, -1, itemDelimiterA, delimiterLength, NULL, NULL);

    TRACE("%p, %s\n", pStm, fClearDirty ? "TRUE" : "FALSE");

    res=IStream_Write(pStm,&delimiterLength,sizeof(DWORD),NULL);
    res=IStream_Write(pStm,itemDelimiterA,delimiterLength * sizeof(CHAR),NULL);
    res=IStream_Write(pStm,&nameLength,sizeof(DWORD),NULL);
    res=IStream_Write(pStm,itemNameA,nameLength * sizeof(CHAR),NULL);

    HeapFree(GetProcessHeap(), 0, itemNameA);
    HeapFree(GetProcessHeap(), 0, itemDelimiterA);

    return res;
}

/******************************************************************************
 *        ItemMoniker_GetSizeMax
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_GetSizeMax(IMoniker* iface,
                                                 ULARGE_INTEGER* pcbSize)/* Pointer to size of stream needed to save object */
{
    ItemMonikerImpl *This = (ItemMonikerImpl *)iface;
    DWORD delimiterLength=lstrlenW(This->itemDelimiter)+1;
    DWORD nameLength=lstrlenW(This->itemName)+1;

    TRACE("(%p,%p)\n",iface,pcbSize);

    if (!pcbSize)
        return E_POINTER;

    /* for more details see ItemMonikerImpl_Save comments */

    pcbSize->u.LowPart =  sizeof(DWORD) + /* DWORD which contains delimiter length */
                        delimiterLength*4 + /* item delimiter string */
                        sizeof(DWORD) + /* DWORD which contains item name length */
                        nameLength*4 + /* item name string */
                        18; /* strange, but true */
    pcbSize->u.HighPart=0;

    return S_OK;
}

/******************************************************************************
 *                  ItemMoniker_BindToObject
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_BindToObject(IMoniker* iface,
                                                   IBindCtx* pbc,
                                                   IMoniker* pmkToLeft,
                                                   REFIID riid,
                                                   VOID** ppvResult)
{
    ItemMonikerImpl *This = (ItemMonikerImpl *)iface;

    HRESULT   res;
    IID    refid=IID_IOleItemContainer;
    IOleItemContainer *poic=0;

    TRACE("(%p,%p,%p,%s,%p)\n",iface,pbc,pmkToLeft,debugstr_guid(riid),ppvResult);

    if(ppvResult ==NULL)
        return E_POINTER;

    if(pmkToLeft==NULL)
        return E_INVALIDARG;

    *ppvResult=0;

    res=IMoniker_BindToObject(pmkToLeft,pbc,NULL,&refid,(void**)&poic);

    if (SUCCEEDED(res)){

        res=IOleItemContainer_GetObject(poic,This->itemName,BINDSPEED_MODERATE,pbc,riid,ppvResult);

        IOleItemContainer_Release(poic);
    }

    return res;
}

/******************************************************************************
 *        ItemMoniker_BindToStorage
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_BindToStorage(IMoniker* iface,
                                                    IBindCtx* pbc,
                                                    IMoniker* pmkToLeft,
                                                    REFIID riid,
                                                    VOID** ppvResult)
{
    ItemMonikerImpl *This = (ItemMonikerImpl *)iface;

    HRESULT   res;
    IOleItemContainer *poic=0;

    TRACE("(%p,%p,%p,%s,%p)\n",iface,pbc,pmkToLeft,debugstr_guid(riid),ppvResult);

    *ppvResult=0;

    if(pmkToLeft==NULL)
        return E_INVALIDARG;

    res=IMoniker_BindToObject(pmkToLeft,pbc,NULL,&IID_IOleItemContainer,(void**)&poic);

    if (SUCCEEDED(res)){

        res=IOleItemContainer_GetObjectStorage(poic,This->itemName,pbc,riid,ppvResult);

        IOleItemContainer_Release(poic);
    }

    return res;
}

/******************************************************************************
 *        ItemMoniker_Reduce
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_Reduce(IMoniker* iface,
                                             IBindCtx* pbc,
                                             DWORD dwReduceHowFar,
                                             IMoniker** ppmkToLeft,
                                             IMoniker** ppmkReduced)
{
    TRACE("(%p,%p,%d,%p,%p)\n",iface,pbc,dwReduceHowFar,ppmkToLeft,ppmkReduced);

    if (ppmkReduced==NULL)
        return E_POINTER;

    ItemMonikerImpl_AddRef(iface);

    *ppmkReduced=iface;

    return MK_S_REDUCED_TO_SELF;
}
/******************************************************************************
 *        ItemMoniker_ComposeWith
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_ComposeWith(IMoniker* iface,
                                                  IMoniker* pmkRight,
                                                  BOOL fOnlyIfNotGeneric,
                                                  IMoniker** ppmkComposite)
{
    HRESULT res=S_OK;
    DWORD mkSys,mkSys2;
    IEnumMoniker* penumMk=0;
    IMoniker *pmostLeftMk=0;
    IMoniker* tempMkComposite=0;

    TRACE("(%p,%p,%d,%p)\n",iface,pmkRight,fOnlyIfNotGeneric,ppmkComposite);

    if ((ppmkComposite==NULL)||(pmkRight==NULL))
	return E_POINTER;

    *ppmkComposite=0;

    IMoniker_IsSystemMoniker(pmkRight,&mkSys);

    /* If pmkRight is an anti-moniker, the returned moniker is NULL */
    if(mkSys==MKSYS_ANTIMONIKER)
        return res;

    else
        /* if pmkRight is a composite whose leftmost component is an anti-moniker,           */
        /* the returned moniker is the composite after the leftmost anti-moniker is removed. */

         if(mkSys==MKSYS_GENERICCOMPOSITE){

            res=IMoniker_Enum(pmkRight,TRUE,&penumMk);

            if (FAILED(res))
                return res;

            res=IEnumMoniker_Next(penumMk,1,&pmostLeftMk,NULL);

            IMoniker_IsSystemMoniker(pmostLeftMk,&mkSys2);

            if(mkSys2==MKSYS_ANTIMONIKER){

                IMoniker_Release(pmostLeftMk);

                tempMkComposite=iface;
                IMoniker_AddRef(iface);

                while(IEnumMoniker_Next(penumMk,1,&pmostLeftMk,NULL)==S_OK){

                    res=CreateGenericComposite(tempMkComposite,pmostLeftMk,ppmkComposite);

                    IMoniker_Release(tempMkComposite);
                    IMoniker_Release(pmostLeftMk);

                    tempMkComposite=*ppmkComposite;
                    IMoniker_AddRef(tempMkComposite);
                }
                return res;
            }
            else
                return CreateGenericComposite(iface,pmkRight,ppmkComposite);
         }
         /* If pmkRight is not an anti-moniker, the method combines the two monikers into a generic
          composite if fOnlyIfNotGeneric is FALSE; if fOnlyIfNotGeneric is TRUE, the method returns
          a NULL moniker and a return value of MK_E_NEEDGENERIC */
          else
            if (!fOnlyIfNotGeneric)
                return CreateGenericComposite(iface,pmkRight,ppmkComposite);

            else
                return MK_E_NEEDGENERIC;
}

/******************************************************************************
 *        ItemMoniker_Enum
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_Enum(IMoniker* iface,BOOL fForward, IEnumMoniker** ppenumMoniker)
{
    TRACE("(%p,%d,%p)\n",iface,fForward,ppenumMoniker);

    if (ppenumMoniker == NULL)
        return E_POINTER;

    *ppenumMoniker = NULL;

    return S_OK;
}

/******************************************************************************
 *        ItemMoniker_IsEqual
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_IsEqual(IMoniker* iface,IMoniker* pmkOtherMoniker)
{

    CLSID clsid;
    LPOLESTR dispName1,dispName2;
    IBindCtx* bind;
    HRESULT res = S_FALSE;

    TRACE("(%p,%p)\n",iface,pmkOtherMoniker);

    if (!pmkOtherMoniker) return S_FALSE;


    /* check if both are ItemMoniker */
    if(FAILED (IMoniker_GetClassID(pmkOtherMoniker,&clsid))) return S_FALSE;
    if(!IsEqualCLSID(&clsid,&CLSID_ItemMoniker)) return S_FALSE;

    /* check if both displaynames are the same */
    if(SUCCEEDED ((res = CreateBindCtx(0,&bind)))) {
        if(SUCCEEDED (IMoniker_GetDisplayName(iface,bind,NULL,&dispName1))) {
	    if(SUCCEEDED (IMoniker_GetDisplayName(pmkOtherMoniker,bind,NULL,&dispName2))) {
                if(lstrcmpW(dispName1,dispName2)==0) res = S_OK;
                CoTaskMemFree(dispName2);
            }
            CoTaskMemFree(dispName1);
	}
    }
    return res;
}

/******************************************************************************
 *        ItemMoniker_Hash
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_Hash(IMoniker* iface,DWORD* pdwHash)
{
    ItemMonikerImpl *This = (ItemMonikerImpl *)iface;
    DWORD h = 0;
    int  i,len;
    int  off = 0;
    LPOLESTR val;

    if (pdwHash==NULL)
        return E_POINTER;

    val =  This->itemName;
    len = lstrlenW(val);

    for (i = len ; i > 0; i--)
        h = (h * 3) ^ toupperW(val[off++]);

    *pdwHash=h;

    return S_OK;
}

/******************************************************************************
 *        ItemMoniker_IsRunning
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_IsRunning(IMoniker* iface,
                                                IBindCtx* pbc,
                                                IMoniker* pmkToLeft,
                                                IMoniker* pmkNewlyRunning)
{
    IRunningObjectTable* rot;
    HRESULT res;
    IOleItemContainer *poic=0;
    ItemMonikerImpl *This = (ItemMonikerImpl *)iface;

    TRACE("(%p,%p,%p,%p)\n",iface,pbc,pmkToLeft,pmkNewlyRunning);

    /* If pmkToLeft is NULL, this method returns TRUE if pmkNewlyRunning is non-NULL and is equal to this */
    /* moniker. Otherwise, the method checks the ROT to see whether this moniker is running.              */
    if (pmkToLeft==NULL)
        if ((pmkNewlyRunning!=NULL)&&(IMoniker_IsEqual(pmkNewlyRunning,iface)==S_OK))
            return S_OK;
        else {
            if (pbc==NULL)
                return E_INVALIDARG;

            res=IBindCtx_GetRunningObjectTable(pbc,&rot);

            if (FAILED(res))
                return res;

            res = IRunningObjectTable_IsRunning(rot,iface);

            IRunningObjectTable_Release(rot);
        }
    else{

        /* If pmkToLeft is non-NULL, the method calls IMoniker::BindToObject on the pmkToLeft parameter,         */
        /* requesting an IOleItemContainer interface pointer. The method then calls IOleItemContainer::IsRunning,*/
        /* passing the string contained within this moniker. */

        res=IMoniker_BindToObject(pmkToLeft,pbc,NULL,&IID_IOleItemContainer,(void**)&poic);

        if (SUCCEEDED(res)){

            res=IOleItemContainer_IsRunning(poic,This->itemName);

            IOleItemContainer_Release(poic);
        }
    }

    return res;
}

/******************************************************************************
 *        ItemMoniker_GetTimeOfLastChange
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_GetTimeOfLastChange(IMoniker* iface,
                                                          IBindCtx* pbc,
                                                          IMoniker* pmkToLeft,
                                                          FILETIME* pItemTime)
{
    IRunningObjectTable* rot;
    HRESULT res;
    IMoniker *compositeMk;

    TRACE("(%p,%p,%p,%p)\n",iface,pbc,pmkToLeft,pItemTime);

    if (pItemTime==NULL)
        return E_INVALIDARG;

    /* If pmkToLeft is NULL, this method returns MK_E_NOTBINDABLE */
    if (pmkToLeft==NULL)

        return MK_E_NOTBINDABLE;
    else {

        /* Otherwise, the method creates a composite of pmkToLeft and this moniker and uses the ROT to access  */
        /* the time of last change. If the object is not in the ROT, the method calls                          */
        /* IMoniker::GetTimeOfLastChange on the pmkToLeft parameter.                                            */

        res=CreateGenericComposite(pmkToLeft,iface,&compositeMk);

        res=IBindCtx_GetRunningObjectTable(pbc,&rot);

        if (IRunningObjectTable_GetTimeOfLastChange(rot,compositeMk,pItemTime)!=S_OK)

            res=IMoniker_GetTimeOfLastChange(pmkToLeft,pbc,NULL,pItemTime);

        IMoniker_Release(compositeMk);
    }

    return res;
}

/******************************************************************************
 *        ItemMoniker_Inverse
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_Inverse(IMoniker* iface,IMoniker** ppmk)
{
    TRACE("(%p,%p)\n",iface,ppmk);

    if (ppmk==NULL)
        return E_POINTER;

    return CreateAntiMoniker(ppmk);
}

/******************************************************************************
 *        ItemMoniker_CommonPrefixWith
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_CommonPrefixWith(IMoniker* iface,IMoniker* pmkOther,IMoniker** ppmkPrefix)
{
    DWORD mkSys;
    
    TRACE("(%p,%p)\n", pmkOther, ppmkPrefix);

    IMoniker_IsSystemMoniker(pmkOther,&mkSys);
    /* If the other moniker is an item moniker that is equal to this moniker, this method sets *ppmkPrefix */
    /* to this moniker and returns MK_S_US */

    if((mkSys==MKSYS_ITEMMONIKER) && (IMoniker_IsEqual(iface,pmkOther)==S_OK) ){

        *ppmkPrefix=iface;

        IMoniker_AddRef(iface);

        return MK_S_US;
    }
    else
        /* otherwise, the method calls the MonikerCommonPrefixWith function. This function correctly handles */
        /* the case where the other moniker is a generic composite. */
        return MonikerCommonPrefixWith(iface,pmkOther,ppmkPrefix);
}

/******************************************************************************
 *        ItemMoniker_RelativePathTo
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_RelativePathTo(IMoniker* iface,IMoniker* pmOther, IMoniker** ppmkRelPath)
{
    TRACE("(%p,%p,%p)\n",iface,pmOther,ppmkRelPath);

    if (ppmkRelPath==NULL)
        return E_POINTER;

    *ppmkRelPath=0;

    return MK_E_NOTBINDABLE;
}

/******************************************************************************
 *        ItemMoniker_GetDisplayName
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_GetDisplayName(IMoniker* iface,
                                                     IBindCtx* pbc,
                                                     IMoniker* pmkToLeft,
                                                     LPOLESTR *ppszDisplayName)
{
    ItemMonikerImpl *This = (ItemMonikerImpl *)iface;

    TRACE("(%p,%p,%p,%p)\n",iface,pbc,pmkToLeft,ppszDisplayName);

    if (ppszDisplayName==NULL)
        return E_POINTER;

    if (pmkToLeft!=NULL){
        return E_INVALIDARG;
    }

    *ppszDisplayName=CoTaskMemAlloc(sizeof(WCHAR)*(lstrlenW(This->itemDelimiter)+lstrlenW(This->itemName)+1));

    if (*ppszDisplayName==NULL)
        return E_OUTOFMEMORY;

    lstrcpyW(*ppszDisplayName,This->itemDelimiter);
    lstrcatW(*ppszDisplayName,This->itemName);

    TRACE("-- %s\n", debugstr_w(*ppszDisplayName));

    return S_OK;
}

/******************************************************************************
 *        ItemMoniker_ParseDisplayName
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_ParseDisplayName(IMoniker* iface,
                                                       IBindCtx* pbc,
                                                       IMoniker* pmkToLeft,
                                                       LPOLESTR pszDisplayName,
                                                       ULONG* pchEaten,
                                                       IMoniker** ppmkOut)
{
    IOleItemContainer* poic=0;
    IParseDisplayName* ppdn=0;
    LPOLESTR displayName;
    HRESULT res;
    ItemMonikerImpl *This = (ItemMonikerImpl *)iface;

    TRACE("%s\n", debugstr_w(pszDisplayName));

    /* If pmkToLeft is NULL, this method returns MK_E_SYNTAX */
    if (pmkToLeft==NULL)

        return MK_E_SYNTAX;

    else{
        /* Otherwise, the method calls IMoniker::BindToObject on the pmkToLeft parameter, requesting an */
        /* IParseDisplayName interface pointer to the object identified by the moniker, and passes the display */
        /* name to IParseDisplayName::ParseDisplayName */
        res=IMoniker_BindToObject(pmkToLeft,pbc,NULL,&IID_IOleItemContainer,(void**)&poic);

        if (SUCCEEDED(res)){

            res=IOleItemContainer_GetObject(poic,This->itemName,BINDSPEED_MODERATE,pbc,&IID_IParseDisplayName,(void**)&ppdn);

            res=IMoniker_GetDisplayName(iface,pbc,NULL,&displayName);

            res=IParseDisplayName_ParseDisplayName(ppdn,pbc,displayName,pchEaten,ppmkOut);

            IOleItemContainer_Release(poic);
            IParseDisplayName_Release(ppdn);
        }
    }
    return res;
}

/******************************************************************************
 *        ItemMoniker_IsSystemMoniker
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerImpl_IsSystemMoniker(IMoniker* iface,DWORD* pwdMksys)
{
    TRACE("(%p,%p)\n",iface,pwdMksys);

    if (!pwdMksys)
        return E_POINTER;

    (*pwdMksys)=MKSYS_ITEMMONIKER;

    return S_OK;
}

/*******************************************************************************
 *        ItemMonikerIROTData_QueryInterface
 *******************************************************************************/
static HRESULT WINAPI ItemMonikerROTDataImpl_QueryInterface(IROTData *iface,REFIID riid,VOID** ppvObject)
{

    IMoniker *This = impl_from_IROTData(iface);

    TRACE("(%p,%p,%p)\n",iface,riid,ppvObject);

    return ItemMonikerImpl_QueryInterface(This, riid, ppvObject);
}

/***********************************************************************
 *        ItemMonikerIROTData_AddRef
 */
static ULONG   WINAPI ItemMonikerROTDataImpl_AddRef(IROTData *iface)
{
    IMoniker *This = impl_from_IROTData(iface);

    TRACE("(%p)\n",iface);

    return ItemMonikerImpl_AddRef(This);
}

/***********************************************************************
 *        ItemMonikerIROTData_Release
 */
static ULONG   WINAPI ItemMonikerROTDataImpl_Release(IROTData* iface)
{
    IMoniker *This = impl_from_IROTData(iface);

    TRACE("(%p)\n",iface);

    return ItemMonikerImpl_Release(This);
}

/******************************************************************************
 *        ItemMonikerIROTData_GetComparisonData
 ******************************************************************************/
static HRESULT WINAPI ItemMonikerROTDataImpl_GetComparisonData(IROTData* iface,
                                                               BYTE* pbData,
                                                               ULONG cbMax,
                                                               ULONG* pcbData)
{
    IMoniker *This = impl_from_IROTData(iface);
    ItemMonikerImpl *This1 = (ItemMonikerImpl *)This;
    int len = (strlenW(This1->itemName)+1);
    int i;
    LPWSTR pszItemName;
    LPWSTR pszItemDelimiter;

    TRACE("(%p, %u, %p)\n", pbData, cbMax, pcbData);

    *pcbData = sizeof(CLSID) + sizeof(WCHAR) + len * sizeof(WCHAR);
    if (cbMax < *pcbData)
        return E_OUTOFMEMORY;

    /* write CLSID */
    memcpy(pbData, &CLSID_ItemMoniker, sizeof(CLSID));
    /* write delimiter */
    pszItemDelimiter = (LPWSTR)(pbData+sizeof(CLSID));
    *pszItemDelimiter = *This1->itemDelimiter;
    /* write name */
    pszItemName = pszItemDelimiter + 1;
    for (i = 0; i < len; i++)
        pszItemName[i] = toupperW(This1->itemName[i]);

    return S_OK;
}

/********************************************************************************/
/* Virtual function table for the ItemMonikerImpl class which  include IPersist,*/
/* IPersistStream and IMoniker functions.                                       */
static const IMonikerVtbl VT_ItemMonikerImpl =
    {
    ItemMonikerImpl_QueryInterface,
    ItemMonikerImpl_AddRef,
    ItemMonikerImpl_Release,
    ItemMonikerImpl_GetClassID,
    ItemMonikerImpl_IsDirty,
    ItemMonikerImpl_Load,
    ItemMonikerImpl_Save,
    ItemMonikerImpl_GetSizeMax,
    ItemMonikerImpl_BindToObject,
    ItemMonikerImpl_BindToStorage,
    ItemMonikerImpl_Reduce,
    ItemMonikerImpl_ComposeWith,
    ItemMonikerImpl_Enum,
    ItemMonikerImpl_IsEqual,
    ItemMonikerImpl_Hash,
    ItemMonikerImpl_IsRunning,
    ItemMonikerImpl_GetTimeOfLastChange,
    ItemMonikerImpl_Inverse,
    ItemMonikerImpl_CommonPrefixWith,
    ItemMonikerImpl_RelativePathTo,
    ItemMonikerImpl_GetDisplayName,
    ItemMonikerImpl_ParseDisplayName,
    ItemMonikerImpl_IsSystemMoniker
};

/********************************************************************************/
/* Virtual function table for the IROTData class.                               */
static const IROTDataVtbl VT_ROTDataImpl =
{
    ItemMonikerROTDataImpl_QueryInterface,
    ItemMonikerROTDataImpl_AddRef,
    ItemMonikerROTDataImpl_Release,
    ItemMonikerROTDataImpl_GetComparisonData
};

/******************************************************************************
 *         ItemMoniker_Construct (local function)
 *******************************************************************************/
static HRESULT ItemMonikerImpl_Construct(ItemMonikerImpl* This, LPCOLESTR lpszDelim,LPCOLESTR lpszItem)
{

    int sizeStr1=lstrlenW(lpszItem), sizeStr2;
    static const OLECHAR emptystr[1];
    LPCOLESTR	delim;

    TRACE("(%p,%s,%s)\n",This,debugstr_w(lpszDelim),debugstr_w(lpszItem));

    /* Initialize the virtual function table. */
    This->lpvtbl1      = &VT_ItemMonikerImpl;
    This->lpvtbl2      = &VT_ROTDataImpl;
    This->ref          = 0;
    This->pMarshal     = NULL;

    This->itemName=HeapAlloc(GetProcessHeap(),0,sizeof(WCHAR)*(sizeStr1+1));
    if (!This->itemName)
	return E_OUTOFMEMORY;
    lstrcpyW(This->itemName,lpszItem);

    if (!lpszDelim)
	FIXME("lpszDelim is NULL. Using empty string which is possibly wrong.\n");

    delim = lpszDelim ? lpszDelim : emptystr;

    sizeStr2=lstrlenW(delim);
    This->itemDelimiter=HeapAlloc(GetProcessHeap(),0,sizeof(WCHAR)*(sizeStr2+1));
    if (!This->itemDelimiter) {
	HeapFree(GetProcessHeap(),0,This->itemName);
	return E_OUTOFMEMORY;
    }
    lstrcpyW(This->itemDelimiter,delim);
    return S_OK;
}

/******************************************************************************
 *        ItemMoniker_Destroy (local function)
 *******************************************************************************/
static HRESULT ItemMonikerImpl_Destroy(ItemMonikerImpl* This)
{
    TRACE("(%p)\n",This);

    if (This->pMarshal) IUnknown_Release(This->pMarshal);
    HeapFree(GetProcessHeap(),0,This->itemName);
    HeapFree(GetProcessHeap(),0,This->itemDelimiter);
    HeapFree(GetProcessHeap(),0,This);

    return S_OK;
}

/******************************************************************************
 *        CreateItemMoniker	[OLE32.@]
 ******************************************************************************/
HRESULT WINAPI CreateItemMoniker(LPCOLESTR lpszDelim,LPCOLESTR  lpszItem, LPMONIKER * ppmk)
{
    ItemMonikerImpl* newItemMoniker;
    HRESULT        hr;

    TRACE("(%s,%s,%p)\n",debugstr_w(lpszDelim),debugstr_w(lpszItem),ppmk);

    newItemMoniker = HeapAlloc(GetProcessHeap(), 0, sizeof(ItemMonikerImpl));

    if (!newItemMoniker)
        return STG_E_INSUFFICIENTMEMORY;

    hr = ItemMonikerImpl_Construct(newItemMoniker,lpszDelim,lpszItem);

    if (FAILED(hr)){

        HeapFree(GetProcessHeap(),0,newItemMoniker);
    return hr;
    }

    return ItemMonikerImpl_QueryInterface((IMoniker*)newItemMoniker,&IID_IMoniker,(void**)ppmk);
}

static HRESULT WINAPI ItemMonikerCF_QueryInterface(LPCLASSFACTORY iface,
                                                  REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;
    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IClassFactory))
    {
        *ppv = iface;
        IUnknown_AddRef(iface);
        return S_OK;
    }
    return E_NOINTERFACE;
}

static ULONG WINAPI ItemMonikerCF_AddRef(LPCLASSFACTORY iface)
{
    return 2; /* non-heap based object */
}

static ULONG WINAPI ItemMonikerCF_Release(LPCLASSFACTORY iface)
{
    return 1; /* non-heap based object */
}

static HRESULT WINAPI ItemMonikerCF_CreateInstance(LPCLASSFACTORY iface,
    LPUNKNOWN pUnk, REFIID riid, LPVOID *ppv)
{
    ItemMonikerImpl* newItemMoniker;
    HRESULT  hr;
    static const WCHAR wszEmpty[] = { 0 };

    TRACE("(%p, %s, %p)\n", pUnk, debugstr_guid(riid), ppv);

    *ppv = NULL;

    if (pUnk)
        return CLASS_E_NOAGGREGATION;

    newItemMoniker = HeapAlloc(GetProcessHeap(), 0, sizeof(ItemMonikerImpl));
    if (!newItemMoniker)
        return E_OUTOFMEMORY;

    hr = ItemMonikerImpl_Construct(newItemMoniker, wszEmpty, wszEmpty);

    if (SUCCEEDED(hr))
	hr = ItemMonikerImpl_QueryInterface((IMoniker*)newItemMoniker, riid, ppv);
    if (FAILED(hr))
        HeapFree(GetProcessHeap(),0,newItemMoniker);

    return hr;
}

static HRESULT WINAPI ItemMonikerCF_LockServer(LPCLASSFACTORY iface, BOOL fLock)
{
    FIXME("(%d), stub!\n",fLock);
    return S_OK;
}

static const IClassFactoryVtbl ItemMonikerCFVtbl =
{
    ItemMonikerCF_QueryInterface,
    ItemMonikerCF_AddRef,
    ItemMonikerCF_Release,
    ItemMonikerCF_CreateInstance,
    ItemMonikerCF_LockServer
};
static const IClassFactoryVtbl *ItemMonikerCF = &ItemMonikerCFVtbl;

HRESULT ItemMonikerCF_Create(REFIID riid, LPVOID *ppv)
{
    return IClassFactory_QueryInterface((IClassFactory *)&ItemMonikerCF, riid, ppv);
}
