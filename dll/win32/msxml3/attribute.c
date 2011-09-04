/*
 *    DOM Attribute implementation
 *
 * Copyright 2006 Huw Davies
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

#define COBJMACROS

#include "config.h"

#include <stdarg.h>
#include "windef.h"
#include "winbase.h"
#include "winuser.h"
#include "ole2.h"
#include "msxml2.h"

#include "msxml_private.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(msxml);

#ifdef HAVE_LIBXML2

typedef struct _domattr
{
    xmlnode node;
    const struct IXMLDOMAttributeVtbl *lpVtbl;
    LONG ref;
} domattr;

static inline domattr *impl_from_IXMLDOMAttribute( IXMLDOMAttribute *iface )
{
    return (domattr *)((char*)iface - FIELD_OFFSET(domattr, lpVtbl));
}

static HRESULT WINAPI domattr_QueryInterface(
    IXMLDOMAttribute *iface,
    REFIID riid,
    void** ppvObject )
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    TRACE("(%p)->(%s %p)\n", This, debugstr_guid(riid), ppvObject);

    if ( IsEqualGUID( riid, &IID_IXMLDOMAttribute ) ||
         IsEqualGUID( riid, &IID_IDispatch ) ||
         IsEqualGUID( riid, &IID_IUnknown ) )
    {
        *ppvObject = iface;
    }
    else if ( IsEqualGUID( riid, &IID_IXMLDOMNode ) )
    {
        *ppvObject = IXMLDOMNode_from_impl(&This->node);
    }
    else
    {
        FIXME("Unsupported interface %s\n", debugstr_guid(riid));
        return E_NOINTERFACE;
    }

    IXMLDOMText_AddRef((IUnknown*)*ppvObject);
    return S_OK;
}

static ULONG WINAPI domattr_AddRef(
    IXMLDOMAttribute *iface )
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return InterlockedIncrement( &This->ref );
}

static ULONG WINAPI domattr_Release(
    IXMLDOMAttribute *iface )
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    ULONG ref;

    ref = InterlockedDecrement( &This->ref );
    if ( ref == 0 )
    {
        destroy_xmlnode(&This->node);
        heap_free( This );
    }

    return ref;
}

static HRESULT WINAPI domattr_GetTypeInfoCount(
    IXMLDOMAttribute *iface,
    UINT* pctinfo )
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );

    TRACE("(%p)->(%p)\n", This, pctinfo);

    *pctinfo = 1;

    return S_OK;
}

static HRESULT WINAPI domattr_GetTypeInfo(
    IXMLDOMAttribute *iface,
    UINT iTInfo, LCID lcid,
    ITypeInfo** ppTInfo )
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    HRESULT hr;

    TRACE("(%p)->(%u %u %p)\n", This, iTInfo, lcid, ppTInfo);

    hr = get_typeinfo(IXMLDOMAttribute_tid, ppTInfo);

    return hr;
}

static HRESULT WINAPI domattr_GetIDsOfNames(
    IXMLDOMAttribute *iface,
    REFIID riid, LPOLESTR* rgszNames,
    UINT cNames, LCID lcid, DISPID* rgDispId )
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    ITypeInfo *typeinfo;
    HRESULT hr;

    TRACE("(%p)->(%s %p %u %u %p)\n", This, debugstr_guid(riid), rgszNames, cNames,
          lcid, rgDispId);

    if(!rgszNames || cNames == 0 || !rgDispId)
        return E_INVALIDARG;

    hr = get_typeinfo(IXMLDOMAttribute_tid, &typeinfo);
    if(SUCCEEDED(hr))
    {
        hr = ITypeInfo_GetIDsOfNames(typeinfo, rgszNames, cNames, rgDispId);
        ITypeInfo_Release(typeinfo);
    }

    return hr;
}

static HRESULT WINAPI domattr_Invoke(
    IXMLDOMAttribute *iface,
    DISPID dispIdMember, REFIID riid, LCID lcid,
    WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
    EXCEPINFO* pExcepInfo, UINT* puArgErr )
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    ITypeInfo *typeinfo;
    HRESULT hr;

    TRACE("(%p)->(%d %s %d %d %p %p %p %p)\n", This, dispIdMember, debugstr_guid(riid),
          lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);

    hr = get_typeinfo(IXMLDOMAttribute_tid, &typeinfo);
    if(SUCCEEDED(hr))
    {
        hr = ITypeInfo_Invoke(typeinfo, &(This->lpVtbl), dispIdMember, wFlags, pDispParams,
                pVarResult, pExcepInfo, puArgErr);
        ITypeInfo_Release(typeinfo);
    }
    return hr;
}

static HRESULT WINAPI domattr_get_nodeName(
    IXMLDOMAttribute *iface,
    BSTR* p )
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_nodeName( IXMLDOMNode_from_impl(&This->node), p );
}

static HRESULT WINAPI domattr_get_nodeValue(
    IXMLDOMAttribute *iface,
    VARIANT* var1 )
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_nodeValue( IXMLDOMNode_from_impl(&This->node), var1 );
}

static HRESULT WINAPI domattr_put_nodeValue(
    IXMLDOMAttribute *iface,
    VARIANT var1 )
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_put_nodeValue( IXMLDOMNode_from_impl(&This->node), var1 );
}

static HRESULT WINAPI domattr_get_nodeType(
    IXMLDOMAttribute *iface,
    DOMNodeType* domNodeType )
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_nodeType( IXMLDOMNode_from_impl(&This->node), domNodeType );
}

static HRESULT WINAPI domattr_get_parentNode(
    IXMLDOMAttribute *iface,
    IXMLDOMNode** parent )
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    TRACE("(%p)->(%p)\n", This, parent);
    if (!parent) return E_INVALIDARG;
    *parent = NULL;
    return S_FALSE;
}

static HRESULT WINAPI domattr_get_childNodes(
    IXMLDOMAttribute *iface,
    IXMLDOMNodeList** outList)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_childNodes( IXMLDOMNode_from_impl(&This->node), outList );
}

static HRESULT WINAPI domattr_get_firstChild(
    IXMLDOMAttribute *iface,
    IXMLDOMNode** domNode)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_firstChild( IXMLDOMNode_from_impl(&This->node), domNode );
}

static HRESULT WINAPI domattr_get_lastChild(
    IXMLDOMAttribute *iface,
    IXMLDOMNode** domNode)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_lastChild( IXMLDOMNode_from_impl(&This->node), domNode );
}

static HRESULT WINAPI domattr_get_previousSibling(
    IXMLDOMAttribute *iface,
    IXMLDOMNode** domNode)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_previousSibling( IXMLDOMNode_from_impl(&This->node), domNode );
}

static HRESULT WINAPI domattr_get_nextSibling(
    IXMLDOMAttribute *iface,
    IXMLDOMNode** domNode)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_nextSibling( IXMLDOMNode_from_impl(&This->node), domNode );
}

static HRESULT WINAPI domattr_get_attributes(
    IXMLDOMAttribute *iface,
    IXMLDOMNamedNodeMap** attributeMap)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_attributes( IXMLDOMNode_from_impl(&This->node), attributeMap );
}

static HRESULT WINAPI domattr_insertBefore(
    IXMLDOMAttribute *iface,
    IXMLDOMNode* newNode, VARIANT var1,
    IXMLDOMNode** outOldNode)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_insertBefore( IXMLDOMNode_from_impl(&This->node), newNode, var1, outOldNode );
}

static HRESULT WINAPI domattr_replaceChild(
    IXMLDOMAttribute *iface,
    IXMLDOMNode* newNode,
    IXMLDOMNode* oldNode,
    IXMLDOMNode** outOldNode)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_replaceChild( IXMLDOMNode_from_impl(&This->node), newNode, oldNode, outOldNode );
}

static HRESULT WINAPI domattr_removeChild(
    IXMLDOMAttribute *iface,
    IXMLDOMNode* domNode, IXMLDOMNode** oldNode)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_removeChild( IXMLDOMNode_from_impl(&This->node), domNode, oldNode );
}

static HRESULT WINAPI domattr_appendChild(
    IXMLDOMAttribute *iface,
    IXMLDOMNode* newNode, IXMLDOMNode** outNewNode)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_appendChild( IXMLDOMNode_from_impl(&This->node), newNode, outNewNode );
}

static HRESULT WINAPI domattr_hasChildNodes(
    IXMLDOMAttribute *iface,
    VARIANT_BOOL* pbool)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_hasChildNodes( IXMLDOMNode_from_impl(&This->node), pbool );
}

static HRESULT WINAPI domattr_get_ownerDocument(
    IXMLDOMAttribute *iface,
    IXMLDOMDocument** domDocument)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_ownerDocument( IXMLDOMNode_from_impl(&This->node), domDocument );
}

static HRESULT WINAPI domattr_cloneNode(
    IXMLDOMAttribute *iface,
    VARIANT_BOOL pbool, IXMLDOMNode** outNode)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_cloneNode( IXMLDOMNode_from_impl(&This->node), pbool, outNode );
}

static HRESULT WINAPI domattr_get_nodeTypeString(
    IXMLDOMAttribute *iface,
    BSTR* p)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_nodeTypeString( IXMLDOMNode_from_impl(&This->node), p );
}

static HRESULT WINAPI domattr_get_text(
    IXMLDOMAttribute *iface,
    BSTR* p)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_text( IXMLDOMNode_from_impl(&This->node), p );
}

static HRESULT WINAPI domattr_put_text(
    IXMLDOMAttribute *iface,
    BSTR p)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_put_text( IXMLDOMNode_from_impl(&This->node), p );
}

static HRESULT WINAPI domattr_get_specified(
    IXMLDOMAttribute *iface,
    VARIANT_BOOL* pbool)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_specified( IXMLDOMNode_from_impl(&This->node), pbool );
}

static HRESULT WINAPI domattr_get_definition(
    IXMLDOMAttribute *iface,
    IXMLDOMNode** domNode)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_definition( IXMLDOMNode_from_impl(&This->node), domNode );
}

static HRESULT WINAPI domattr_get_nodeTypedValue(
    IXMLDOMAttribute *iface,
    VARIANT* var1)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_nodeTypedValue( IXMLDOMNode_from_impl(&This->node), var1 );
}

static HRESULT WINAPI domattr_put_nodeTypedValue(
    IXMLDOMAttribute *iface,
    VARIANT var1)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_put_nodeTypedValue( IXMLDOMNode_from_impl(&This->node), var1 );
}

static HRESULT WINAPI domattr_get_dataType(
    IXMLDOMAttribute *iface,
    VARIANT* var1)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_dataType( IXMLDOMNode_from_impl(&This->node), var1 );
}

static HRESULT WINAPI domattr_put_dataType(
    IXMLDOMAttribute *iface,
    BSTR p)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_put_dataType( IXMLDOMNode_from_impl(&This->node), p );
}

static HRESULT WINAPI domattr_get_xml(
    IXMLDOMAttribute *iface,
    BSTR* p)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_xml( IXMLDOMNode_from_impl(&This->node), p );
}

static HRESULT WINAPI domattr_transformNode(
    IXMLDOMAttribute *iface,
    IXMLDOMNode* domNode, BSTR* p)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_transformNode( IXMLDOMNode_from_impl(&This->node), domNode, p );
}

static HRESULT WINAPI domattr_selectNodes(
    IXMLDOMAttribute *iface,
    BSTR p, IXMLDOMNodeList** outList)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_selectNodes( IXMLDOMNode_from_impl(&This->node), p, outList );
}

static HRESULT WINAPI domattr_selectSingleNode(
    IXMLDOMAttribute *iface,
    BSTR p, IXMLDOMNode** outNode)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_selectSingleNode( IXMLDOMNode_from_impl(&This->node), p, outNode );
}

static HRESULT WINAPI domattr_get_parsed(
    IXMLDOMAttribute *iface,
    VARIANT_BOOL* pbool)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_parsed( IXMLDOMNode_from_impl(&This->node), pbool );
}

static HRESULT WINAPI domattr_get_namespaceURI(
    IXMLDOMAttribute *iface,
    BSTR* p)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_namespaceURI( IXMLDOMNode_from_impl(&This->node), p );
}

static HRESULT WINAPI domattr_get_prefix(
    IXMLDOMAttribute *iface,
    BSTR* p)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_prefix( IXMLDOMNode_from_impl(&This->node), p );
}

static HRESULT WINAPI domattr_get_baseName(
    IXMLDOMAttribute *iface,
    BSTR* p)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_baseName( IXMLDOMNode_from_impl(&This->node), p );
}

static HRESULT WINAPI domattr_transformNodeToObject(
    IXMLDOMAttribute *iface,
    IXMLDOMNode* domNode, VARIANT var1)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_transformNodeToObject( IXMLDOMNode_from_impl(&This->node), domNode, var1 );
}

static HRESULT WINAPI domattr_get_name(
    IXMLDOMAttribute *iface,
    BSTR *p)
{
    /* name property returns the same value as nodeName */
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_nodeName( IXMLDOMNode_from_impl(&This->node), p );
}

static HRESULT WINAPI domattr_get_value(
    IXMLDOMAttribute *iface,
    VARIANT *var1)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_get_nodeValue( IXMLDOMNode_from_impl(&This->node), var1 );
}

static HRESULT WINAPI domattr_put_value(
    IXMLDOMAttribute *iface,
    VARIANT var1)
{
    domattr *This = impl_from_IXMLDOMAttribute( iface );
    return IXMLDOMNode_put_nodeValue( IXMLDOMNode_from_impl(&This->node), var1 );
}

static const struct IXMLDOMAttributeVtbl domattr_vtbl =
{
    domattr_QueryInterface,
    domattr_AddRef,
    domattr_Release,
    domattr_GetTypeInfoCount,
    domattr_GetTypeInfo,
    domattr_GetIDsOfNames,
    domattr_Invoke,
    domattr_get_nodeName,
    domattr_get_nodeValue,
    domattr_put_nodeValue,
    domattr_get_nodeType,
    domattr_get_parentNode,
    domattr_get_childNodes,
    domattr_get_firstChild,
    domattr_get_lastChild,
    domattr_get_previousSibling,
    domattr_get_nextSibling,
    domattr_get_attributes,
    domattr_insertBefore,
    domattr_replaceChild,
    domattr_removeChild,
    domattr_appendChild,
    domattr_hasChildNodes,
    domattr_get_ownerDocument,
    domattr_cloneNode,
    domattr_get_nodeTypeString,
    domattr_get_text,
    domattr_put_text,
    domattr_get_specified,
    domattr_get_definition,
    domattr_get_nodeTypedValue,
    domattr_put_nodeTypedValue,
    domattr_get_dataType,
    domattr_put_dataType,
    domattr_get_xml,
    domattr_transformNode,
    domattr_selectNodes,
    domattr_selectSingleNode,
    domattr_get_parsed,
    domattr_get_namespaceURI,
    domattr_get_prefix,
    domattr_get_baseName,
    domattr_transformNodeToObject,
    domattr_get_name,
    domattr_get_value,
    domattr_put_value
};

IUnknown* create_attribute( xmlNodePtr attribute )
{
    domattr *This;

    This = heap_alloc( sizeof *This );
    if ( !This )
        return NULL;

    This->lpVtbl = &domattr_vtbl;
    This->ref = 1;

    init_xmlnode(&This->node, attribute, (IUnknown*)&This->lpVtbl, NULL);

    return (IUnknown*) &This->lpVtbl;
}

#endif
