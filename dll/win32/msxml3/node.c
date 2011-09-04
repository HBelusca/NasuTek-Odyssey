/*
 *    Node implementation
 *
 * Copyright 2005 Mike McCormack
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

#define COBJMACROS

#include <stdarg.h>
#include <assert.h>
#include "windef.h"
#include "winbase.h"
#include "winuser.h"
#include "winnls.h"
#include "ole2.h"
#include "msxml2.h"

#include "msxml_private.h"

#ifdef HAVE_LIBXML2
# include <libxml/HTMLtree.h>
#endif

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(msxml);

#ifdef HAVE_LIBXML2

static const WCHAR szBinBase64[]  = {'b','i','n','.','b','a','s','e','6','4',0};
static const WCHAR szString[]     = {'s','t','r','i','n','g',0};
static const WCHAR szNumber[]     = {'n','u','m','b','e','r',0};
static const WCHAR szInt[]        = {'I','n','t',0};
static const WCHAR szFixed[]      = {'F','i','x','e','d','.','1','4','.','4',0};
static const WCHAR szBoolean[]    = {'B','o','o','l','e','a','n',0};
static const WCHAR szDateTime[]   = {'d','a','t','e','T','i','m','e',0};
static const WCHAR szDateTimeTZ[] = {'d','a','t','e','T','i','m','e','.','t','z',0};
static const WCHAR szDate[]       = {'D','a','t','e',0};
static const WCHAR szTime[]       = {'T','i','m','e',0};
static const WCHAR szTimeTZ[]     = {'T','i','m','e','.','t','z',0};
static const WCHAR szI1[]         = {'i','1',0};
static const WCHAR szI2[]         = {'i','2',0};
static const WCHAR szI4[]         = {'i','4',0};
static const WCHAR szIU1[]        = {'u','i','1',0};
static const WCHAR szIU2[]        = {'u','i','2',0};
static const WCHAR szIU4[]        = {'u','i','4',0};
static const WCHAR szR4[]         = {'r','4',0};
static const WCHAR szR8[]         = {'r','8',0};
static const WCHAR szFloat[]      = {'f','l','o','a','t',0};
static const WCHAR szUUID[]       = {'u','u','i','d',0};
static const WCHAR szBinHex[]     = {'b','i','n','.','h','e','x',0};

xmlNodePtr xmlNodePtr_from_domnode( IXMLDOMNode *iface, xmlElementType type )
{
    xmlnode *This;

    if ( !iface )
        return NULL;
    This = impl_from_IXMLDOMNode( iface );
    if ( !This->node )
        return NULL;
    if ( type && This->node->type != type )
        return NULL;
    return This->node;
}

static HRESULT WINAPI xmlnode_QueryInterface(
    IXMLDOMNode *iface,
    REFIID riid,
    void** ppvObject )
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );

    TRACE("(%p)->(%s %p)\n", This, debugstr_guid(riid), ppvObject);

    if(This->pUnkOuter)
        return IUnknown_QueryInterface(This->pUnkOuter, riid, ppvObject);

    if (IsEqualGUID(riid, &IID_IUnknown)) {
        *ppvObject = iface;
    }else if (IsEqualGUID( riid, &IID_IDispatch) ||
              IsEqualGUID( riid, &IID_IXMLDOMNode)) {
        *ppvObject = &This->lpVtbl;
    }else  {
        FIXME("interface %s not implemented\n", debugstr_guid(riid));
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    IUnknown_AddRef( (IUnknown*)*ppvObject );
    return S_OK;
}

static ULONG WINAPI xmlnode_AddRef(
    IXMLDOMNode *iface )
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );

    if(This->pUnkOuter)
        return IUnknown_AddRef(This->pUnkOuter);

    return InterlockedIncrement(&This->ref);
}

static ULONG WINAPI xmlnode_Release(
    IXMLDOMNode *iface )
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    LONG ref;

    if(This->pUnkOuter)
        return IUnknown_Release(This->pUnkOuter);

    ref = InterlockedDecrement( &This->ref );
    if(!ref) {
        destroy_xmlnode(This);
        heap_free( This );
    }

    return ref;
}

static HRESULT WINAPI xmlnode_GetTypeInfoCount(
    IXMLDOMNode *iface,
    UINT* pctinfo )
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );

    TRACE("(%p)->(%p)\n", This, pctinfo);

    *pctinfo = 1;

    return S_OK;
}

static HRESULT WINAPI xmlnode_GetTypeInfo(
    IXMLDOMNode *iface,
    UINT iTInfo,
    LCID lcid,
    ITypeInfo** ppTInfo )
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    HRESULT hr;

    TRACE("(%p)->(%u %u %p)\n", This, iTInfo, lcid, ppTInfo);

    hr = get_typeinfo(IXMLDOMNode_tid, ppTInfo);

    return hr;
}

static HRESULT WINAPI xmlnode_GetIDsOfNames(
    IXMLDOMNode *iface,
    REFIID riid,
    LPOLESTR* rgszNames,
    UINT cNames,
    LCID lcid,
    DISPID* rgDispId )
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );

    ITypeInfo *typeinfo;
    HRESULT hr;

    TRACE("(%p)->(%s %p %u %u %p)\n", This, debugstr_guid(riid), rgszNames, cNames,
          lcid, rgDispId);

    if(!rgszNames || cNames == 0 || !rgDispId)
        return E_INVALIDARG;

    hr = get_typeinfo(IXMLDOMNode_tid, &typeinfo);
    if(SUCCEEDED(hr))
    {
        hr = ITypeInfo_GetIDsOfNames(typeinfo, rgszNames, cNames, rgDispId);
        ITypeInfo_Release(typeinfo);
    }

    return hr;
}

static HRESULT WINAPI xmlnode_Invoke(
    IXMLDOMNode *iface,
    DISPID dispIdMember,
    REFIID riid,
    LCID lcid,
    WORD wFlags,
    DISPPARAMS* pDispParams,
    VARIANT* pVarResult,
    EXCEPINFO* pExcepInfo,
    UINT* puArgErr )
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    ITypeInfo *typeinfo;
    HRESULT hr;

    TRACE("(%p)->(%d %s %d %d %p %p %p %p)\n", This, dispIdMember, debugstr_guid(riid),
          lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);

    hr = get_typeinfo(IXMLDOMNode_tid, &typeinfo);
    if(SUCCEEDED(hr))
    {
        hr = ITypeInfo_Invoke(typeinfo, &(This->lpVtbl), dispIdMember, wFlags, pDispParams,
                pVarResult, pExcepInfo, puArgErr);
        ITypeInfo_Release(typeinfo);
    }

    return hr;
}

static HRESULT WINAPI xmlnode_get_nodeName(
    IXMLDOMNode *iface,
    BSTR* name)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    const xmlChar *str;

    TRACE("(%p)->(%p)\n", This, name );

    if (!name)
        return E_INVALIDARG;

    if ( !This->node )
        return E_FAIL;

    switch( This->node->type )
    {
    case XML_CDATA_SECTION_NODE:
        str = (const xmlChar*) "#cdata-section";
        break;
    case XML_COMMENT_NODE:
        str = (const xmlChar*) "#comment";
        break;
    case XML_DOCUMENT_FRAG_NODE:
        str = (const xmlChar*) "#document-fragment";
        break;
    case XML_TEXT_NODE:
        str = (const xmlChar*) "#text";
        break;
    case XML_DOCUMENT_NODE:
        str = (const xmlChar*) "#document";
        break;
    case XML_ATTRIBUTE_NODE:
    case XML_ELEMENT_NODE:
    case XML_PI_NODE:
        str = This->node->name;
        break;
    default:
        FIXME("nodeName not mapped correctly (%d)\n", This->node->type);
        str = This->node->name;
        break;
    }

    *name = bstr_from_xmlChar( str );
    if (!*name)
        return S_FALSE;

    return S_OK;
}

static HRESULT WINAPI xmlnode_get_nodeValue(
    IXMLDOMNode *iface,
    VARIANT* value)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    HRESULT r = S_FALSE;

    TRACE("(%p)->(%p)\n", This, value);

    if(!value)
        return E_INVALIDARG;

    V_BSTR(value) = NULL;
    V_VT(value) = VT_NULL;

    switch ( This->node->type )
    {
    case XML_CDATA_SECTION_NODE:
    case XML_COMMENT_NODE:
    case XML_PI_NODE:
    case XML_ATTRIBUTE_NODE:
      {
        xmlChar *content = xmlNodeGetContent(This->node);
        V_VT(value) = VT_BSTR;
        V_BSTR(value) = bstr_from_xmlChar( content );
        xmlFree(content);
        r = S_OK;
        break;
      }
    case XML_TEXT_NODE:
        V_VT(value) = VT_BSTR;
        V_BSTR(value) = bstr_from_xmlChar( This->node->content );
        r = S_OK;
        break;
    case XML_ELEMENT_NODE:
    case XML_DOCUMENT_NODE:
        /* these seem to return NULL */
        break;

    default:
        FIXME("node %p type %d\n", This, This->node->type);
    }
 
    TRACE("%p returned %s\n", This, debugstr_w( V_BSTR(value) ) );

    return r;
}

static HRESULT WINAPI xmlnode_put_nodeValue(
    IXMLDOMNode *iface,
    VARIANT value)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    HRESULT hr;

    TRACE("%p type(%d)\n", This, This->node->type);

    /* Document, Document Fragment, Document Type, Element,
       Entity, Entity Reference, Notation aren't supported. */
    switch ( This->node->type )
    {
    case XML_ATTRIBUTE_NODE:
    case XML_CDATA_SECTION_NODE:
    case XML_COMMENT_NODE:
    case XML_PI_NODE:
    case XML_TEXT_NODE:
    {
        VARIANT string_value;
        xmlChar *str;

        VariantInit(&string_value);
        hr = VariantChangeType(&string_value, &value, 0, VT_BSTR);
        if(FAILED(hr))
        {
            VariantClear(&string_value);
            WARN("Couldn't convert to VT_BSTR\n");
            return hr;
        }

        str = xmlChar_from_wchar(V_BSTR(&string_value));
        VariantClear(&string_value);

        xmlNodeSetContent(This->node, str);
        heap_free(str);
        hr = S_OK;
        break;
    }
    default:
        /* Do nothing for unsupported types. */
        hr = E_FAIL;
        break;
    }

    return hr;
}

static HRESULT WINAPI xmlnode_get_nodeType(
    IXMLDOMNode *iface,
    DOMNodeType* type)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );

    TRACE("(%p)->(%p)\n", This, type);

    assert( (int)NODE_ELEMENT  == (int)XML_ELEMENT_NODE );
    assert( (int)NODE_NOTATION == (int)XML_NOTATION_NODE );

    *type = This->node->type;

    return S_OK;
}

static HRESULT get_node(
    xmlnode *This,
    const char *name,
    xmlNodePtr node,
    IXMLDOMNode **out )
{
    TRACE("(%p)->(%s %p %p)\n", This, name, node, out );

    if ( !out )
        return E_INVALIDARG;

    /* if we don't have a doc, use our parent. */
    if(node && !node->doc && node->parent)
        node->doc = node->parent->doc;

    *out = create_node( node );
    if (!*out)
        return S_FALSE;
    return S_OK;
}

static HRESULT WINAPI xmlnode_get_parentNode(
    IXMLDOMNode *iface,
    IXMLDOMNode** parent)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    return get_node( This, "parent", This->node->parent, parent );
}

static HRESULT WINAPI xmlnode_get_childNodes(
    IXMLDOMNode *iface,
    IXMLDOMNodeList** childList)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );

    TRACE("(%p)->(%p)\n", This, childList );

    if ( !childList )
        return E_INVALIDARG;

    *childList = create_children_nodelist(This->node);
    if (*childList == NULL)
        return E_OUTOFMEMORY;

    return S_OK;
}

static HRESULT WINAPI xmlnode_get_firstChild(
    IXMLDOMNode *iface,
    IXMLDOMNode** firstChild)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    TRACE("(%p)->(%p)\n", This, firstChild);
    return get_node( This, "firstChild", This->node->children, firstChild );
}

static HRESULT WINAPI xmlnode_get_lastChild(
    IXMLDOMNode *iface,
    IXMLDOMNode** lastChild)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );

    TRACE("(%p)->(%p)\n", This, lastChild );

    if (!lastChild)
        return E_INVALIDARG;

    switch( This->node->type )
    {
    /* CDATASection, Comment, PI and Text Nodes do not support lastChild */
    case XML_TEXT_NODE:
    case XML_CDATA_SECTION_NODE:
    case XML_PI_NODE:
    case XML_COMMENT_NODE:
        *lastChild = NULL;
        return S_FALSE;
    default:
        return get_node( This, "lastChild", This->node->last, lastChild );
    }
}

static HRESULT WINAPI xmlnode_get_previousSibling(
    IXMLDOMNode *iface,
    IXMLDOMNode** previousSibling)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );

    TRACE("(%p)->(%p)\n", This, previousSibling );

    if (!previousSibling)
        return E_INVALIDARG;

    switch( This->node->type )
    {
    /* Attribute, Document and Document Fragment Nodes do not support previousSibling */
    case XML_DOCUMENT_NODE:
    case XML_DOCUMENT_FRAG_NODE:
    case XML_ATTRIBUTE_NODE:
        *previousSibling = NULL;
        return S_FALSE;
    default:
        return get_node( This, "previous", This->node->prev, previousSibling );
    }
}

static HRESULT WINAPI xmlnode_get_nextSibling(
    IXMLDOMNode *iface,
    IXMLDOMNode** nextSibling)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );

    TRACE("(%p)->(%p)\n", This, nextSibling );

    if (!nextSibling)
        return E_INVALIDARG;

    switch( This->node->type )
    {
    /* Attribute, Document and Document Fragment Nodes do not support nextSibling */
    case XML_DOCUMENT_NODE:
    case XML_DOCUMENT_FRAG_NODE:
    case XML_ATTRIBUTE_NODE:
        *nextSibling = NULL;
        return S_FALSE;
    default:
        return get_node( This, "next", This->node->next, nextSibling );
    }
}

static HRESULT WINAPI xmlnode_get_attributes(
    IXMLDOMNode *iface,
    IXMLDOMNamedNodeMap** attributeMap)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    TRACE("(%p)->(%p)\n", This, attributeMap);

    if (!attributeMap)
        return E_INVALIDARG;

    switch( This->node->type )
    {
    /* Attribute, CDataSection, Comment, Documents, Documents Fragments,
       Entity and Text Nodes does not support get_attributes */
    case XML_ATTRIBUTE_NODE:
    case XML_CDATA_SECTION_NODE:
    case XML_COMMENT_NODE:
    case XML_DOCUMENT_NODE:
    case XML_DOCUMENT_FRAG_NODE:
    case XML_ENTITY_NODE:
    case XML_ENTITY_REF_NODE:
    case XML_TEXT_NODE:
        *attributeMap = NULL;
        return S_FALSE;
    default:
        *attributeMap = create_nodemap( iface );
        return S_OK;
    }
}

static HRESULT WINAPI xmlnode_insertBefore(
    IXMLDOMNode *iface,
    IXMLDOMNode* newChild,
    VARIANT refChild,
    IXMLDOMNode** outNewChild)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    xmlNodePtr before_node, new_child_node;
    IXMLDOMNode *before = NULL, *new;
    HRESULT hr;

    TRACE("(%p)->(%p var %p)\n",This,newChild,outNewChild);

    if (!newChild)
        return E_INVALIDARG;

    switch(V_VT(&refChild))
    {
    case VT_EMPTY:
    case VT_NULL:
        break;

    case VT_UNKNOWN:
        hr = IUnknown_QueryInterface(V_UNKNOWN(&refChild), &IID_IXMLDOMNode, (LPVOID)&before);
        if(FAILED(hr)) return hr;
        break;

    case VT_DISPATCH:
        hr = IDispatch_QueryInterface(V_DISPATCH(&refChild), &IID_IXMLDOMNode, (LPVOID)&before);
        if(FAILED(hr)) return hr;
        break;

    default:
        FIXME("refChild var type %x\n", V_VT(&refChild));
        return E_FAIL;
    }

    IXMLDOMNode_QueryInterface(newChild, &IID_IXMLDOMNode, (LPVOID)&new);
    new_child_node = impl_from_IXMLDOMNode(new)->node;
    TRACE("new_child_node %p This->node %p\n", new_child_node, This->node);

    if(!new_child_node->parent)
        if(xmldoc_remove_orphan(new_child_node->doc, new_child_node) != S_OK)
            WARN("%p is not an orphan of %p\n", new_child_node, new_child_node->doc);

    if(before)
    {
        before_node = impl_from_IXMLDOMNode(before)->node;
        xmlAddPrevSibling(before_node, new_child_node);
        IXMLDOMNode_Release(before);
    }
    else
    {
        xmlAddChild(This->node, new_child_node);
    }

    IXMLDOMNode_Release(new);
    IXMLDOMNode_AddRef(newChild);
    if(outNewChild)
        *outNewChild = newChild;

    TRACE("ret S_OK\n");
    return S_OK;
}

static HRESULT WINAPI xmlnode_replaceChild(
    IXMLDOMNode *iface,
    IXMLDOMNode* newChild,
    IXMLDOMNode* oldChild,
    IXMLDOMNode** outOldChild)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    xmlNode *old_child_ptr, *new_child_ptr;
    xmlDocPtr leaving_doc;
    xmlNode *my_ancestor;
    IXMLDOMNode *realOldChild;
    HRESULT hr;

    TRACE("(%p)->(%p %p %p)\n", This, newChild, oldChild, outOldChild);

    /* Do not believe any documentation telling that newChild == NULL
       means removal. It does certainly *not* apply to msxml3! */
    if(!newChild || !oldChild)
        return E_INVALIDARG;

    if(outOldChild)
        *outOldChild = NULL;

    hr = IXMLDOMNode_QueryInterface(oldChild,&IID_IXMLDOMNode,(LPVOID*)&realOldChild);
    if(FAILED(hr))
        return hr;

    old_child_ptr = impl_from_IXMLDOMNode(realOldChild)->node;
    IXMLDOMNode_Release(realOldChild);
    if(old_child_ptr->parent != This->node)
    {
        WARN("childNode %p is not a child of %p\n", oldChild, iface);
        return E_INVALIDARG;
    }

    new_child_ptr = impl_from_IXMLDOMNode(newChild)->node;
    my_ancestor = This->node;
    while(my_ancestor)
    {
        if(my_ancestor == new_child_ptr)
        {
            WARN("tried to create loop\n");
            return E_FAIL;
        }
        my_ancestor = my_ancestor->parent;
    }

    if(!new_child_ptr->parent)
        if(xmldoc_remove_orphan(new_child_ptr->doc, new_child_ptr) != S_OK)
            WARN("%p is not an orphan of %p\n", new_child_ptr, new_child_ptr->doc);

    leaving_doc = new_child_ptr->doc;
    xmldoc_add_ref(old_child_ptr->doc);
    xmlReplaceNode(old_child_ptr, new_child_ptr);
    xmldoc_release(leaving_doc);

    xmldoc_add_orphan(old_child_ptr->doc, old_child_ptr);

    if(outOldChild)
    {
        IXMLDOMNode_AddRef(oldChild);
        *outOldChild = oldChild;
    }

    return S_OK;
}

static HRESULT WINAPI xmlnode_removeChild(
    IXMLDOMNode *iface,
    IXMLDOMNode* childNode,
    IXMLDOMNode** oldChild)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    xmlNode *child_node_ptr;
    HRESULT hr;
    IXMLDOMNode *child;

    TRACE("(%p)->(%p %p)\n", This, childNode, oldChild);

    if(!childNode) return E_INVALIDARG;

    if(oldChild)
        *oldChild = NULL;

    hr = IXMLDOMNode_QueryInterface(childNode, &IID_IXMLDOMNode, (LPVOID)&child);
    if(FAILED(hr))
        return hr;

    child_node_ptr = impl_from_IXMLDOMNode(child)->node;
    if(child_node_ptr->parent != This->node)
    {
        WARN("childNode %p is not a child of %p\n", childNode, iface);
        IXMLDOMNode_Release(child);
        return E_INVALIDARG;
    }

    xmlUnlinkNode(child_node_ptr);

    IXMLDOMNode_Release(child);

    if(oldChild)
    {
        IXMLDOMNode_AddRef(childNode);
        *oldChild = childNode;
    }

    return S_OK;
}

static HRESULT WINAPI xmlnode_appendChild(
    IXMLDOMNode *iface,
    IXMLDOMNode* newChild,
    IXMLDOMNode** outNewChild)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    DOMNodeType type;
    VARIANT var;
    HRESULT hr;

    TRACE("(%p)->(%p %p)\n", This, newChild, outNewChild);

    hr = IXMLDOMNode_get_nodeType(newChild, &type);
    if(FAILED(hr) || type == NODE_ATTRIBUTE) {
        if(outNewChild) *outNewChild = NULL;
        return E_FAIL;
    }

    VariantInit(&var);
    return IXMLDOMNode_insertBefore(iface, newChild, var, outNewChild);
}

static HRESULT WINAPI xmlnode_hasChildNodes(
    IXMLDOMNode *iface,
    VARIANT_BOOL* hasChild)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );

    TRACE("(%p)->(%p)\n", This, hasChild);

    if (!hasChild)
        return E_INVALIDARG;
    if (!This->node->children)
    {
        *hasChild = VARIANT_FALSE;
        return S_FALSE;
    }

    *hasChild = VARIANT_TRUE;
    return S_OK;
}

static HRESULT WINAPI xmlnode_get_ownerDocument(
    IXMLDOMNode *iface,
    IXMLDOMDocument** DOMDocument)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );

    TRACE("(%p)->(%p)\n", This, DOMDocument);

    return DOMDocument_create_from_xmldoc(This->node->doc, (IXMLDOMDocument2**)DOMDocument);
}

static HRESULT WINAPI xmlnode_cloneNode(
    IXMLDOMNode *iface,
    VARIANT_BOOL deep,
    IXMLDOMNode** cloneRoot)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    xmlNodePtr pClone = NULL;
    IXMLDOMNode *pNode = NULL;

    TRACE("(%p)->(%d %p)\n", This, deep, cloneRoot);

    if(!cloneRoot)
        return E_INVALIDARG;

    pClone = xmlCopyNode(This->node, deep ? 1 : 2);
    if(pClone)
    {
        pClone->doc = This->node->doc;
        xmldoc_add_orphan(pClone->doc, pClone);

        pNode = create_node(pClone);
        if(!pNode)
        {
            ERR("Copy failed\n");
            return E_FAIL;
        }

        *cloneRoot = pNode;
    }
    else
    {
        ERR("Copy failed\n");
        return E_FAIL;
    }

    return S_OK;
}

static HRESULT WINAPI xmlnode_get_nodeTypeString(
    IXMLDOMNode *iface,
    BSTR* xmlnodeType)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    const xmlChar *str;

    TRACE("(%p)->(%p)\n", This, xmlnodeType );

    if (!xmlnodeType)
        return E_INVALIDARG;

    if ( !This->node )
        return E_FAIL;

    switch( This->node->type )
    {
    case XML_ATTRIBUTE_NODE:
        str = (const xmlChar*) "attribute";
        break;
    case XML_CDATA_SECTION_NODE:
        str = (const xmlChar*) "cdatasection";
        break;
    case XML_COMMENT_NODE:
        str = (const xmlChar*) "comment";
        break;
    case XML_DOCUMENT_NODE:
        str = (const xmlChar*) "document";
        break;
    case XML_DOCUMENT_FRAG_NODE:
        str = (const xmlChar*) "documentfragment";
        break;
    case XML_ELEMENT_NODE:
        str = (const xmlChar*) "element";
        break;
    case XML_ENTITY_NODE:
        str = (const xmlChar*) "entity";
        break;
    case XML_ENTITY_REF_NODE:
        str = (const xmlChar*) "entityreference";
        break;
    case XML_NOTATION_NODE:
        str = (const xmlChar*) "notation";
        break;
    case XML_PI_NODE:
        str = (const xmlChar*) "processinginstruction";
        break;
    case XML_TEXT_NODE:
        str = (const xmlChar*) "text";
        break;
    default:
        FIXME("Unknown node type (%d)\n", This->node->type);
        str = This->node->name;
        break;
    }

    *xmlnodeType = bstr_from_xmlChar( str );
    if (!*xmlnodeType)
        return S_FALSE;

    return S_OK;
}

static HRESULT WINAPI xmlnode_get_text(
    IXMLDOMNode *iface,
    BSTR* text)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    BSTR str = NULL;
    xmlChar *pContent;

    TRACE("(%p, type %d)->(%p)\n", This, This->node->type, text);

    if ( !text )
        return E_INVALIDARG;

    pContent = xmlNodeGetContent((xmlNodePtr)This->node);
    if(pContent)
    {
        str = bstr_from_xmlChar(pContent);
        xmlFree(pContent);
    }

    /* Always return a string. */
    if (!str) str = SysAllocStringLen( NULL, 0 );

    TRACE("%p %s\n", This, debugstr_w(str) );
    *text = str;
 
    return S_OK;
}

static HRESULT WINAPI xmlnode_put_text(
    IXMLDOMNode *iface,
    BSTR text)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    xmlChar *str, *str2;

    TRACE("(%p)->(%s)\n", This, debugstr_w(text));

    switch(This->node->type)
    {
    case XML_DOCUMENT_NODE:
        return E_FAIL;
    default:
        break;
    }

    str = xmlChar_from_wchar(text);

    /* Escape the string. */
    str2 = xmlEncodeEntitiesReentrant(This->node->doc, str);
    heap_free(str);

    xmlNodeSetContent(This->node, str2);
    xmlFree(str2);

    return S_OK;
}

static HRESULT WINAPI xmlnode_get_specified(
    IXMLDOMNode *iface,
    VARIANT_BOOL* isSpecified)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    FIXME("(%p)->(%p) stub!\n", This, isSpecified);
    *isSpecified = VARIANT_TRUE;
    return S_OK;
}

static HRESULT WINAPI xmlnode_get_definition(
    IXMLDOMNode *iface,
    IXMLDOMNode** definitionNode)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    FIXME("(%p)->(%p)\n", This, definitionNode);
    return E_NOTIMPL;
}

static inline BYTE hex_to_byte(xmlChar c)
{
    if(c <= '9') return c-'0';
    if(c <= 'F') return c-'A'+10;
    return c-'a'+10;
}

static inline BYTE base64_to_byte(xmlChar c)
{
    if(c == '+') return 62;
    if(c == '/') return 63;
    if(c <= '9') return c-'0'+52;
    if(c <= 'Z') return c-'A';
    return c-'a'+26;
}

static inline HRESULT VARIANT_from_xmlChar(xmlChar *str, VARIANT *v, BSTR type)
{
    if(!type || !lstrcmpiW(type, szString) ||
            !lstrcmpiW(type, szNumber) || !lstrcmpiW(type, szUUID))
    {
        V_VT(v) = VT_BSTR;
        V_BSTR(v) = bstr_from_xmlChar(str);

        if(!V_BSTR(v))
            return E_OUTOFMEMORY;
    }
    else if(!lstrcmpiW(type, szDateTime) || !lstrcmpiW(type, szDateTimeTZ) ||
            !lstrcmpiW(type, szDate) || !lstrcmpiW(type, szTime) ||
            !lstrcmpiW(type, szTimeTZ))
    {
        VARIANT src;
        WCHAR *p, *e;
        SYSTEMTIME st;
        DOUBLE date = 0.0;

        st.wYear = 1899;
        st.wMonth = 12;
        st.wDay = 30;
        st.wDayOfWeek = st.wHour = st.wMinute = st.wSecond = st.wMilliseconds = 0;

        V_VT(&src) = VT_BSTR;
        V_BSTR(&src) = bstr_from_xmlChar(str);

        if(!V_BSTR(&src))
            return E_OUTOFMEMORY;

        p = V_BSTR(&src);
        e = p + SysStringLen(V_BSTR(&src));

        if(p+4<e && *(p+4)=='-') /* parse date (yyyy-mm-dd) */
        {
            st.wYear = atoiW(p);
            st.wMonth = atoiW(p+5);
            st.wDay = atoiW(p+8);
            p += 10;

            if(*p == 'T') p++;
        }

        if(p+2<e && *(p+2)==':') /* parse time (hh:mm:ss.?) */
        {
            st.wHour = atoiW(p);
            st.wMinute = atoiW(p+3);
            st.wSecond = atoiW(p+6);
            p += 8;

            if(*p == '.')
            {
                p++;
                while(isdigitW(*p)) p++;
            }
        }

        SystemTimeToVariantTime(&st, &date);
        V_VT(v) = VT_DATE;
        V_DATE(v) = date;

        if(*p == '+') /* parse timezone offset (+hh:mm) */
            V_DATE(v) += (DOUBLE)atoiW(p+1)/24 + (DOUBLE)atoiW(p+4)/1440;
        else if(*p == '-') /* parse timezone offset (-hh:mm) */
            V_DATE(v) -= (DOUBLE)atoiW(p+1)/24 + (DOUBLE)atoiW(p+4)/1440;

        VariantClear(&src);
    }
    else if(!lstrcmpiW(type, szBinHex))
    {
        SAFEARRAYBOUND sab;
        int i, len;

        len = xmlStrlen(str)/2;
        sab.lLbound = 0;
        sab.cElements = len;

        V_VT(v) = (VT_ARRAY|VT_UI1);
        V_ARRAY(v) = SafeArrayCreate(VT_UI1, 1, &sab);

        if(!V_ARRAY(v))
            return E_OUTOFMEMORY;

        for(i=0; i<len; i++)
            ((BYTE*)V_ARRAY(v)->pvData)[i] = (hex_to_byte(str[2*i])<<4)
                + hex_to_byte(str[2*i+1]);
    }
    else if(!lstrcmpiW(type, szBinBase64))
    {
        SAFEARRAYBOUND sab;
        int i, len;

        len  = xmlStrlen(str);
        if(str[len-2] == '=') i = 2;
        else if(str[len-1] == '=') i = 1;
        else i = 0;

        sab.lLbound = 0;
        sab.cElements = len/4*3-i;

        V_VT(v) = (VT_ARRAY|VT_UI1);
        V_ARRAY(v) = SafeArrayCreate(VT_UI1, 1, &sab);

        if(!V_ARRAY(v))
            return E_OUTOFMEMORY;

        for(i=0; i<len/4; i++)
        {
            ((BYTE*)V_ARRAY(v)->pvData)[3*i] = (base64_to_byte(str[4*i])<<2)
                + (base64_to_byte(str[4*i+1])>>4);
            if(3*i+1 < sab.cElements)
                ((BYTE*)V_ARRAY(v)->pvData)[3*i+1] = (base64_to_byte(str[4*i+1])<<4)
                    + (base64_to_byte(str[4*i+2])>>2);
            if(3*i+2 < sab.cElements)
                ((BYTE*)V_ARRAY(v)->pvData)[3*i+2] = (base64_to_byte(str[4*i+2])<<6)
                    + base64_to_byte(str[4*i+3]);
        }
    }
    else
    {
        VARIANT src;
        HRESULT hres;

        if(!lstrcmpiW(type, szInt) || !lstrcmpiW(type, szI4))
            V_VT(v) = VT_I4;
        else if(!lstrcmpiW(type, szFixed))
            V_VT(v) = VT_CY;
        else if(!lstrcmpiW(type, szBoolean))
            V_VT(v) = VT_BOOL;
        else if(!lstrcmpiW(type, szI1))
            V_VT(v) = VT_I1;
        else if(!lstrcmpiW(type, szI2))
            V_VT(v) = VT_I2;
        else if(!lstrcmpiW(type, szIU1))
            V_VT(v) = VT_UI1;
        else if(!lstrcmpiW(type, szIU2))
            V_VT(v) = VT_UI2;
        else if(!lstrcmpiW(type, szIU4))
            V_VT(v) = VT_UI4;
        else if(!lstrcmpiW(type, szR4))
            V_VT(v) = VT_R4;
        else if(!lstrcmpiW(type, szR8) || !lstrcmpiW(type, szFloat))
            V_VT(v) = VT_R8;
        else
        {
            FIXME("Type handling not yet implemented\n");
            V_VT(v) = VT_BSTR;
        }

        V_VT(&src) = VT_BSTR;
        V_BSTR(&src) = bstr_from_xmlChar(str);

        if(!V_BSTR(&src))
            return E_OUTOFMEMORY;

        hres = VariantChangeTypeEx(v, &src, MAKELCID(MAKELANGID(
                        LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT),0, V_VT(v));
        VariantClear(&src);
        return hres;
    }

    return S_OK;
}

static HRESULT WINAPI xmlnode_get_nodeTypedValue(
    IXMLDOMNode *iface,
    VARIANT* typedValue)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    VARIANT type;
    xmlChar *content;
    HRESULT hres = S_FALSE;

    TRACE("(%p)->(%p)\n", This, typedValue);

    if(!typedValue)
        return E_INVALIDARG;

    V_VT(typedValue) = VT_NULL;

    if(This->node->type == XML_ELEMENT_NODE ||
            This->node->type == XML_TEXT_NODE ||
            This->node->type == XML_ENTITY_REF_NODE)
        hres = IXMLDOMNode_get_dataType(iface, &type);

    if(hres != S_OK && This->node->type != XML_ELEMENT_NODE)
        return IXMLDOMNode_get_nodeValue(iface, typedValue);

    content = xmlNodeGetContent(This->node);
    hres = VARIANT_from_xmlChar(content, typedValue,
            hres==S_OK ? V_BSTR(&type) : NULL);
    xmlFree(content);
    VariantClear(&type);

    return hres;
}

static HRESULT WINAPI xmlnode_put_nodeTypedValue(
    IXMLDOMNode *iface,
    VARIANT typedValue)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    FIXME("%p\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI xmlnode_get_dataType(
    IXMLDOMNode *iface,
    VARIANT* dataTypeName)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    xmlChar *pVal;

    TRACE("(%p)->(%p)\n", This, dataTypeName);

    if(!dataTypeName)
        return E_INVALIDARG;

    /* Attribute, CDATA Section, Comment, Document, Document Fragment,
        Entity, Notation, PI, and Text Node are non-typed. */
    V_BSTR(dataTypeName) = NULL;
    V_VT(dataTypeName) = VT_NULL;

    switch ( This->node->type )
    {
    case XML_ELEMENT_NODE:
        pVal = xmlGetNsProp(This->node, (const xmlChar*)"dt",
                            (const xmlChar*)"urn:schemas-microsoft-com:datatypes");
        if (pVal)
        {
            V_VT(dataTypeName) = VT_BSTR;
            V_BSTR(dataTypeName) = bstr_from_xmlChar( pVal );
            xmlFree(pVal);
        }
        break;
    case XML_ENTITY_REF_NODE:
        FIXME("XML_ENTITY_REF_NODE should return a valid value.\n");
        break;
    default:
        TRACE("Type %d returning NULL\n", This->node->type);
    }

    /* non-typed nodes return S_FALSE */
    if(V_VT(dataTypeName) == VT_NULL)
    {
        return S_FALSE;
    }

    return S_OK;
}

static HRESULT WINAPI xmlnode_put_dataType(
    IXMLDOMNode *iface,
    BSTR dataTypeName)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    HRESULT hr = E_FAIL;

    TRACE("(%p)->(%s)\n", This, debugstr_w(dataTypeName));

    if(dataTypeName == NULL)
        return E_INVALIDARG;

    /* An example of this is. The Text in the node needs to be a 0 or 1 for a boolean type.
       This applies to changing types (string->bool) or setting a new one
     */
    FIXME("Need to Validate the data before allowing a type to be set.\n");

    /* Check all supported types. */
    if(lstrcmpiW(dataTypeName,szString) == 0  ||
       lstrcmpiW(dataTypeName,szNumber) == 0  ||
       lstrcmpiW(dataTypeName,szUUID) == 0    ||
       lstrcmpiW(dataTypeName,szInt) == 0     ||
       lstrcmpiW(dataTypeName,szI4) == 0      ||
       lstrcmpiW(dataTypeName,szFixed) == 0   ||
       lstrcmpiW(dataTypeName,szBoolean) == 0 ||
       lstrcmpiW(dataTypeName,szDateTime) == 0 ||
       lstrcmpiW(dataTypeName,szDateTimeTZ) == 0 ||
       lstrcmpiW(dataTypeName,szDate) == 0    ||
       lstrcmpiW(dataTypeName,szTime) == 0    ||
       lstrcmpiW(dataTypeName,szTimeTZ) == 0  ||
       lstrcmpiW(dataTypeName,szI1) == 0      ||
       lstrcmpiW(dataTypeName,szI2) == 0      ||
       lstrcmpiW(dataTypeName,szIU1) == 0     ||
       lstrcmpiW(dataTypeName,szIU2) == 0     ||
       lstrcmpiW(dataTypeName,szIU4) == 0     ||
       lstrcmpiW(dataTypeName,szR4) == 0      ||
       lstrcmpiW(dataTypeName,szR8) == 0      ||
       lstrcmpiW(dataTypeName,szFloat) == 0   ||
       lstrcmpiW(dataTypeName,szBinHex) == 0  ||
       lstrcmpiW(dataTypeName,szBinBase64) == 0)
    {
        xmlNsPtr pNS = NULL;
        xmlAttrPtr pAttr = NULL;
        xmlChar* str = xmlChar_from_wchar(dataTypeName);

        pAttr = xmlHasNsProp(This->node, (const xmlChar*)"dt",
                            (const xmlChar*)"urn:schemas-microsoft-com:datatypes");
        if (pAttr)
        {
            pAttr = xmlSetNsProp(This->node, pAttr->ns, (const xmlChar*)"dt", str);

            hr = S_OK;
        }
        else
        {
            pNS = xmlNewNs(This->node, (const xmlChar*)"urn:schemas-microsoft-com:datatypes", (const xmlChar*)"dt");
            if(pNS)
            {
                pAttr = xmlNewNsProp(This->node, pNS, (const xmlChar*)"dt", str);
                if(pAttr)
                {
                    xmlAddChild(This->node, (xmlNodePtr)pAttr);

                    hr = S_OK;
                }
                else
                    ERR("Failed to create Attribute\n");
            }
            else
                ERR("Failed to Create Namepsace\n");
        }
        heap_free( str );
    }

    return hr;
}

static BSTR EnsureCorrectEOL(BSTR sInput)
{
    int nNum = 0;
    BSTR sNew;
    int nLen;
    int i;

    nLen = lstrlenW(sInput);
    /* Count line endings */
    for(i=0; i < nLen; i++)
    {
        if(sInput[i] == '\n')
            nNum++;
    }

    TRACE("len=%d, num=%d\n", nLen, nNum);

    /* Add linefeed as needed */
    if(nNum > 0)
    {
        int nPlace = 0;
        sNew = SysAllocStringLen(NULL, nLen + nNum+1);
        for(i=0; i < nLen; i++)
        {
            if(sInput[i] == '\n')
            {
                sNew[i+nPlace] = '\r';
                nPlace++;
            }
            sNew[i+nPlace] = sInput[i];
        }

        SysFreeString(sInput);
    }
    else
    {
        sNew = sInput;
    }

    TRACE("len %d\n", lstrlenW(sNew));

    return sNew;
}

/* Removes encoding information and last character (nullbyte) */
static BSTR EnsureNoEncoding(BSTR sInput)
{
    static const WCHAR wszEncoding[] = {'e','n','c','o','d','i','n','g','='};
    BSTR sNew;
    WCHAR *pBeg, *pEnd;

    pBeg = sInput;
    while(*pBeg != '\n' && memcmp(pBeg, wszEncoding, sizeof(wszEncoding)))
        pBeg++;

    if(*pBeg == '\n')
    {
        SysReAllocStringLen(&sInput, sInput, SysStringLen(sInput)-1);
        return sInput;
    }
    pBeg--;

    pEnd = pBeg + sizeof(wszEncoding)/sizeof(WCHAR) + 2;
    while(*pEnd != '\"') pEnd++;
    pEnd++;

    sNew = SysAllocStringLen(NULL,
            pBeg-sInput + SysStringLen(sInput)-(pEnd-sInput)-1);
    memcpy(sNew, sInput, (pBeg-sInput)*sizeof(WCHAR));
    memcpy(&sNew[pBeg-sInput], pEnd, (SysStringLen(sInput)-(pEnd-sInput)-1)*sizeof(WCHAR));

    SysFreeString(sInput);
    return sNew;
}

/*
 * We are trying to replicate the same behaviour as msxml by converting
 * line endings to \r\n and using indents as \t. The problem is that msxml
 * only formats nodes that have a line ending. Using libxml we cannot
 * reproduce behaviour exactly.
 *
 */
static HRESULT WINAPI xmlnode_get_xml(
    IXMLDOMNode *iface,
    BSTR* xmlString)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    xmlBufferPtr pXmlBuf;
    xmlNodePtr xmldecl;
    int nSize;

    TRACE("(%p %d)->(%p)\n", This, This->node->type, xmlString);

    if(!xmlString)
        return E_INVALIDARG;

    *xmlString = NULL;

    xmldecl = xmldoc_unlink_xmldecl( This->node->doc );

    pXmlBuf = xmlBufferCreate();
    if(pXmlBuf)
    {
        nSize = xmlNodeDump(pXmlBuf, This->node->doc, This->node, 0, 1);
        if(nSize > 0)
        {
            const xmlChar *pContent;
            BSTR bstrContent;

            /* Attribute Nodes return a space in front of their name */
            pContent = xmlBufferContent(pXmlBuf);
            if( ((const char*)pContent)[0] == ' ')
                bstrContent = bstr_from_xmlChar(pContent+1);
            else
                bstrContent = bstr_from_xmlChar(pContent);

            switch(This->node->type)
            {
                case XML_ELEMENT_NODE:
                    *xmlString = EnsureCorrectEOL(bstrContent);
                    break;
                case XML_DOCUMENT_NODE:
                    *xmlString = EnsureCorrectEOL(bstrContent);
                    *xmlString = EnsureNoEncoding(*xmlString);
                    break;
                default:
                    *xmlString = bstrContent;
            }
        }

        xmlBufferFree(pXmlBuf);
    }

    xmldoc_link_xmldecl( This->node->doc, xmldecl );

    /* Always returns a string. */
    if(*xmlString == NULL)  *xmlString = SysAllocStringLen( NULL, 0 );

    return S_OK;
}

static HRESULT WINAPI xmlnode_transformNode(
    IXMLDOMNode *iface,
    IXMLDOMNode* styleSheet,
    BSTR* xmlString)
{
#ifdef SONAME_LIBXSLT
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    xmlnode *pStyleSheet = NULL;
    xsltStylesheetPtr xsltSS = NULL;
    xmlDocPtr result = NULL;
    IXMLDOMNode *ssNew;

    TRACE("(%p)->(%p %p)\n", This, styleSheet, xmlString);

    if (!libxslt_handle)
        return E_NOTIMPL;
    if(!styleSheet || !xmlString)
        return E_INVALIDARG;

    *xmlString = NULL;

    if(IXMLDOMNode_QueryInterface(styleSheet, &IID_IXMLDOMNode, (LPVOID)&ssNew) == S_OK)
    {
        pStyleSheet = impl_from_IXMLDOMNode( ssNew );

        xsltSS = pxsltParseStylesheetDoc( pStyleSheet->node->doc);
        if(xsltSS)
        {
            result = pxsltApplyStylesheet(xsltSS, This->node->doc, NULL);
            if(result)
            {
                const xmlChar *pContent;

                if(result->type == XML_HTML_DOCUMENT_NODE)
                {
                    xmlOutputBufferPtr	pOutput = xmlAllocOutputBuffer(NULL);
                    if(pOutput)
                    {
                        htmlDocContentDumpOutput(pOutput, result->doc, NULL);
                        pContent = xmlBufferContent(pOutput->buffer);
                        *xmlString = bstr_from_xmlChar(pContent);
                        xmlOutputBufferClose(pOutput);
                    }
                }
                else
                {
                    xmlBufferPtr pXmlBuf;
                    int nSize;

                    pXmlBuf = xmlBufferCreate();
                    if(pXmlBuf)
                    {
                        nSize = xmlNodeDump(pXmlBuf, NULL, (xmlNodePtr)result, 0, 0);
                        if(nSize > 0)
                        {
                            pContent = xmlBufferContent(pXmlBuf);
                            *xmlString = bstr_from_xmlChar(pContent);
                        }
                        xmlBufferFree(pXmlBuf);
                    }
                }
                xmlFreeDoc(result);
            }
            /* libxslt "helpfully" frees the XML document the stylesheet was
               generated from, too */
            xsltSS->doc = NULL;
            pxsltFreeStylesheet(xsltSS);
        }

        IXMLDOMNode_Release(ssNew);
    }

    if(*xmlString == NULL)
        *xmlString = SysAllocStringLen(NULL, 0);

    return S_OK;
#else
    FIXME("libxslt headers were not found at compile time\n");
    return E_NOTIMPL;
#endif
}

static HRESULT WINAPI xmlnode_selectNodes(
    IXMLDOMNode *iface,
    BSTR queryString,
    IXMLDOMNodeList** resultList)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );

    TRACE("(%p)->(%s %p)\n", This, debugstr_w(queryString), resultList );

    return queryresult_create( This->node, queryString, resultList );
}

static HRESULT WINAPI xmlnode_selectSingleNode(
    IXMLDOMNode *iface,
    BSTR queryString,
    IXMLDOMNode** resultNode)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    IXMLDOMNodeList *list;
    HRESULT r;

    TRACE("(%p)->(%s %p)\n", This, debugstr_w(queryString), resultNode );

    *resultNode = NULL;
    r = IXMLDOMNode_selectNodes(iface, queryString, &list);
    if(r == S_OK)
    {
        r = IXMLDOMNodeList_nextNode(list, resultNode);
        IXMLDOMNodeList_Release(list);
    }
    return r;
}

static HRESULT WINAPI xmlnode_get_parsed(
    IXMLDOMNode *iface,
    VARIANT_BOOL* isParsed)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    FIXME("(%p)->(%p) stub!\n", This, isParsed);
    *isParsed = VARIANT_TRUE;
    return S_OK;
}

static HRESULT WINAPI xmlnode_get_namespaceURI(
    IXMLDOMNode *iface,
    BSTR* namespaceURI)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    HRESULT hr = S_FALSE;
    xmlNsPtr *pNSList;

    TRACE("(%p)->(%p)\n", This, namespaceURI );

    if(!namespaceURI)
        return E_INVALIDARG;

    *namespaceURI = NULL;

    pNSList = xmlGetNsList(This->node->doc, This->node);
    if(pNSList)
    {
        *namespaceURI = bstr_from_xmlChar( pNSList[0]->href );

        xmlFree( pNSList );
        hr = S_OK;
    }

    return hr;
}

static HRESULT WINAPI xmlnode_get_prefix(
    IXMLDOMNode *iface,
    BSTR* prefixString)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    HRESULT hr = S_FALSE;
    xmlNsPtr *pNSList;

    TRACE("(%p)->(%p)\n", This, prefixString );

    if(!prefixString)
        return E_INVALIDARG;

    *prefixString = NULL;

    pNSList = xmlGetNsList(This->node->doc, This->node);
    if(pNSList)
    {
        *prefixString = bstr_from_xmlChar( pNSList[0]->prefix );

        xmlFree(pNSList);
        hr = S_OK;
    }

    return hr;
}

static HRESULT WINAPI xmlnode_get_baseName(
    IXMLDOMNode *iface,
    BSTR* nameString)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    BSTR str = NULL;
    HRESULT r = S_FALSE;

    TRACE("(%p)->(%p)\n", This, nameString );

    if ( !nameString )
        return E_INVALIDARG;

    switch ( This->node->type )
    {
    case XML_ELEMENT_NODE:
    case XML_ATTRIBUTE_NODE:
    case XML_PI_NODE:
        str = bstr_from_xmlChar( This->node->name );
        r = S_OK;
        break;
    case XML_TEXT_NODE:
    case XML_COMMENT_NODE:
    case XML_DOCUMENT_NODE:
        break;
    default:
        ERR("Unhandled type %d\n", This->node->type );
        break;
    }

    TRACE("returning %08x str = %s\n", r, debugstr_w( str ) );

    *nameString = str;
    return r;
}

static HRESULT WINAPI xmlnode_transformNodeToObject(
    IXMLDOMNode *iface,
    IXMLDOMNode* stylesheet,
    VARIANT outputObject)
{
    xmlnode *This = impl_from_IXMLDOMNode( iface );
    FIXME("(%p)->(%p)\n", This, stylesheet);
    return E_NOTIMPL;
}

static const struct IXMLDOMNodeVtbl xmlnode_vtbl =
{
    xmlnode_QueryInterface,
    xmlnode_AddRef,
    xmlnode_Release,
    xmlnode_GetTypeInfoCount,
    xmlnode_GetTypeInfo,
    xmlnode_GetIDsOfNames,
    xmlnode_Invoke,
    xmlnode_get_nodeName,
    xmlnode_get_nodeValue,
    xmlnode_put_nodeValue,
    xmlnode_get_nodeType,
    xmlnode_get_parentNode,
    xmlnode_get_childNodes,
    xmlnode_get_firstChild,
    xmlnode_get_lastChild,
    xmlnode_get_previousSibling,
    xmlnode_get_nextSibling,
    xmlnode_get_attributes,
    xmlnode_insertBefore,
    xmlnode_replaceChild,
    xmlnode_removeChild,
    xmlnode_appendChild,
    xmlnode_hasChildNodes,
    xmlnode_get_ownerDocument,
    xmlnode_cloneNode,
    xmlnode_get_nodeTypeString,
    xmlnode_get_text,
    xmlnode_put_text,
    xmlnode_get_specified,
    xmlnode_get_definition,
    xmlnode_get_nodeTypedValue,
    xmlnode_put_nodeTypedValue,
    xmlnode_get_dataType,
    xmlnode_put_dataType,
    xmlnode_get_xml,
    xmlnode_transformNode,
    xmlnode_selectNodes,
    xmlnode_selectSingleNode,
    xmlnode_get_parsed,
    xmlnode_get_namespaceURI,
    xmlnode_get_prefix,
    xmlnode_get_baseName,
    xmlnode_transformNodeToObject,
};

void destroy_xmlnode(xmlnode *This)
{
    if(This->node)
        xmldoc_release(This->node->doc);
}

void init_xmlnode(xmlnode *This, xmlNodePtr node, IUnknown *outer, dispex_static_data_t *dispex_data )
{
    if(node)
        xmldoc_add_ref( node->doc );

    This->lpVtbl = &xmlnode_vtbl;
    This->ref = 1;
    This->node = node;
    This->pUnkOuter = outer;

    if(dispex_data)
        init_dispex(&This->dispex, This->pUnkOuter, dispex_data);
}

IXMLDOMNode *create_node( xmlNodePtr node )
{
    IUnknown *pUnk;
    IXMLDOMNode *ret;
    HRESULT hr;

    if ( !node )
        return NULL;

    TRACE("type %d\n", node->type);
    switch(node->type)
    {
    case XML_ELEMENT_NODE:
        pUnk = create_element( node );
        break;
    case XML_ATTRIBUTE_NODE:
        pUnk = create_attribute( node );
        break;
    case XML_TEXT_NODE:
        pUnk = create_text( node );
        break;
    case XML_CDATA_SECTION_NODE:
        pUnk = create_cdata( node );
        break;
    case XML_ENTITY_REF_NODE:
        pUnk = create_doc_entity_ref( node );
        break;
    case XML_PI_NODE:
        pUnk = create_pi( node );
        break;
    case XML_COMMENT_NODE:
        pUnk = create_comment( node );
        break;
    case XML_DOCUMENT_NODE:
        pUnk = create_domdoc( node );
        break;
    case XML_DOCUMENT_FRAG_NODE:
        pUnk = create_doc_fragment( node );
        break;
    default: {
        xmlnode *new_node;

        FIXME("only creating basic node for type %d\n", node->type);

        new_node = heap_alloc(sizeof(xmlnode));
        if(!new_node)
            return NULL;

        init_xmlnode(new_node, node, NULL, NULL);
        pUnk = (IUnknown*)IXMLDOMNode_from_impl(new_node);
    }
    }

    hr = IUnknown_QueryInterface(pUnk, &IID_IXMLDOMNode, (LPVOID*)&ret);
    IUnknown_Release(pUnk);
    if(FAILED(hr)) return NULL;
    return ret;
}
#endif
