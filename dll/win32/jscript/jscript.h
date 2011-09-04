/*
 * Copyright 2008-2009 Jacek Caban for CodeWeavers
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

#include <stdarg.h>
#include <stdio.h>

#define COBJMACROS

#include "windef.h"
#include "winbase.h"
#include "winuser.h"
#include "ole2.h"
#include "dispex.h"
#include "activscp.h"

#include "resource.h"

#include "wine/unicode.h"
#include "wine/list.h"

#define JSCRIPT_ERROR 0x800A0000

typedef struct _script_ctx_t script_ctx_t;
typedef struct _exec_ctx_t exec_ctx_t;
typedef struct _dispex_prop_t dispex_prop_t;

typedef struct {
    EXCEPINFO ei;
    VARIANT var;
} jsexcept_t;

typedef struct {
    void **blocks;
    DWORD block_cnt;
    DWORD last_block;
    DWORD offset;
    BOOL mark;
    struct list custom_blocks;
} jsheap_t;

void jsheap_init(jsheap_t*);
void *jsheap_alloc(jsheap_t*,DWORD);
void *jsheap_grow(jsheap_t*,void*,DWORD,DWORD);
void jsheap_clear(jsheap_t*);
void jsheap_free(jsheap_t*);
jsheap_t *jsheap_mark(jsheap_t*);

typedef struct DispatchEx DispatchEx;

extern HINSTANCE jscript_hinstance;

#define PROPF_ARGMASK 0x00ff
#define PROPF_METHOD  0x0100
#define PROPF_ENUM    0x0200
#define PROPF_CONSTR  0x0400
#define PROPF_CONST   0x0800

/* NOTE: Keep in sync with names in Object.toString implementation */
typedef enum {
    JSCLASS_NONE,
    JSCLASS_ARRAY,
    JSCLASS_BOOLEAN,
    JSCLASS_DATE,
    JSCLASS_ERROR,
    JSCLASS_FUNCTION,
    JSCLASS_GLOBAL,
    JSCLASS_MATH,
    JSCLASS_NUMBER,
    JSCLASS_OBJECT,
    JSCLASS_REGEXP,
    JSCLASS_STRING,
    JSCLASS_ARGUMENTS
} jsclass_t;

DispatchEx *iface_to_jsdisp(IUnknown*);

typedef struct {
    union {
        IDispatch *disp;
        IDispatchEx *dispex;
        DispatchEx *jsdisp;
    } u;
    DWORD flags;
} vdisp_t;

#define VDISP_DISPEX  0x0001
#define VDISP_JSDISP  0x0002

static inline void vdisp_release(vdisp_t *vdisp)
{
    IDispatch_Release(vdisp->u.disp);
}

static inline BOOL is_jsdisp(vdisp_t *vdisp)
{
    return (vdisp->flags & VDISP_JSDISP) != 0;
}

static inline BOOL is_dispex(vdisp_t *vdisp)
{
    return (vdisp->flags & VDISP_DISPEX) != 0;
}

static inline void set_jsdisp(vdisp_t *vdisp, DispatchEx *jsdisp)
{
    vdisp->u.jsdisp = jsdisp;
    vdisp->flags = VDISP_JSDISP | VDISP_DISPEX;
    IDispatch_AddRef(vdisp->u.disp);
}

static inline void set_disp(vdisp_t *vdisp, IDispatch *disp)
{
    IDispatchEx *dispex;
    DispatchEx *jsdisp;
    HRESULT hres;

    jsdisp = iface_to_jsdisp((IUnknown*)disp);
    if(jsdisp) {
        vdisp->u.jsdisp = jsdisp;
        vdisp->flags = VDISP_JSDISP | VDISP_DISPEX;
        return;
    }

    hres = IDispatch_QueryInterface(disp, &IID_IDispatchEx, (void**)&dispex);
    if(SUCCEEDED(hres)) {
        vdisp->u.dispex = dispex;
        vdisp->flags = VDISP_DISPEX;
        return;
    }

    IDispatch_AddRef(disp);
    vdisp->u.disp = disp;
    vdisp->flags = 0;
}

static inline DispatchEx *get_jsdisp(vdisp_t *vdisp)
{
    return is_jsdisp(vdisp) ? vdisp->u.jsdisp : NULL;
}

typedef HRESULT (*builtin_invoke_t)(script_ctx_t*,vdisp_t*,WORD,DISPPARAMS*,VARIANT*,jsexcept_t*,IServiceProvider*);

typedef struct {
    const WCHAR *name;
    builtin_invoke_t invoke;
    DWORD flags;
} builtin_prop_t;

typedef struct {
    jsclass_t class;
    builtin_prop_t value_prop;
    DWORD props_cnt;
    const builtin_prop_t *props;
    void (*destructor)(DispatchEx*);
    void (*on_put)(DispatchEx*,const WCHAR*);
} builtin_info_t;

struct DispatchEx {
    const IDispatchExVtbl  *lpIDispatchExVtbl;

    LONG ref;

    DWORD buf_size;
    DWORD prop_cnt;
    dispex_prop_t *props;
    script_ctx_t *ctx;

    DispatchEx *prototype;

    const builtin_info_t *builtin_info;
};

#define _IDispatchEx_(x) ((IDispatchEx*) &(x)->lpIDispatchExVtbl)

static inline void jsdisp_release(DispatchEx *jsdisp)
{
    IDispatchEx_Release(_IDispatchEx_(jsdisp));
}

HRESULT create_dispex(script_ctx_t*,const builtin_info_t*,DispatchEx*,DispatchEx**);
HRESULT init_dispex(DispatchEx*,script_ctx_t*,const builtin_info_t*,DispatchEx*);
HRESULT init_dispex_from_constr(DispatchEx*,script_ctx_t*,const builtin_info_t*,DispatchEx*);

HRESULT disp_call(script_ctx_t*,IDispatch*,DISPID,WORD,DISPPARAMS*,VARIANT*,jsexcept_t*,IServiceProvider*);
HRESULT jsdisp_call_value(DispatchEx*,WORD,DISPPARAMS*,VARIANT*,jsexcept_t*,IServiceProvider*);
HRESULT jsdisp_call(DispatchEx*,DISPID,WORD,DISPPARAMS*,VARIANT*,jsexcept_t*,IServiceProvider*);
HRESULT jsdisp_call_name(DispatchEx*,const WCHAR*,WORD,DISPPARAMS*,VARIANT*,jsexcept_t*,IServiceProvider*);
HRESULT disp_propget(script_ctx_t*,IDispatch*,DISPID,VARIANT*,jsexcept_t*,IServiceProvider*);
HRESULT disp_propput(script_ctx_t*,IDispatch*,DISPID,VARIANT*,jsexcept_t*,IServiceProvider*);
HRESULT jsdisp_propget(DispatchEx*,DISPID,VARIANT*,jsexcept_t*,IServiceProvider*);
HRESULT jsdisp_propput_name(DispatchEx*,const WCHAR*,VARIANT*,jsexcept_t*,IServiceProvider*);
HRESULT jsdisp_propput_const(DispatchEx*,const WCHAR*,VARIANT*);
HRESULT jsdisp_propput_idx(DispatchEx*,DWORD,VARIANT*,jsexcept_t*,IServiceProvider*);
HRESULT jsdisp_propget_name(DispatchEx*,LPCWSTR,VARIANT*,jsexcept_t*,IServiceProvider*);
HRESULT jsdisp_get_idx(DispatchEx*,DWORD,VARIANT*,jsexcept_t*,IServiceProvider*);
HRESULT jsdisp_get_id(DispatchEx*,const WCHAR*,DWORD,DISPID*);
HRESULT jsdisp_delete_idx(DispatchEx*,DWORD);

HRESULT create_builtin_function(script_ctx_t*,builtin_invoke_t,const WCHAR*,const builtin_info_t*,DWORD,
        DispatchEx*,DispatchEx**);
HRESULT Function_value(script_ctx_t*,vdisp_t*,WORD,DISPPARAMS*,VARIANT*,jsexcept_t*,IServiceProvider*);

HRESULT throw_eval_error(script_ctx_t*,jsexcept_t*,UINT,const WCHAR*);
HRESULT throw_generic_error(script_ctx_t*,jsexcept_t*,UINT,const WCHAR*);
HRESULT throw_range_error(script_ctx_t*,jsexcept_t*,UINT,const WCHAR*);
HRESULT throw_reference_error(script_ctx_t*,jsexcept_t*,UINT,const WCHAR*);
HRESULT throw_regexp_error(script_ctx_t*,jsexcept_t*,UINT,const WCHAR*);
HRESULT throw_syntax_error(script_ctx_t*,jsexcept_t*,UINT,const WCHAR*);
HRESULT throw_type_error(script_ctx_t*,jsexcept_t*,UINT,const WCHAR*);
HRESULT throw_uri_error(script_ctx_t*,jsexcept_t*,UINT,const WCHAR*);

HRESULT create_object(script_ctx_t*,DispatchEx*,DispatchEx**);
HRESULT create_math(script_ctx_t*,DispatchEx**);
HRESULT create_array(script_ctx_t*,DWORD,DispatchEx**);
HRESULT create_regexp(script_ctx_t*,const WCHAR *,int,DWORD,DispatchEx**);
HRESULT create_regexp_var(script_ctx_t*,VARIANT*,VARIANT*,DispatchEx**);
HRESULT create_string(script_ctx_t*,const WCHAR*,DWORD,DispatchEx**);
HRESULT create_bool(script_ctx_t*,VARIANT_BOOL,DispatchEx**);
HRESULT create_number(script_ctx_t*,VARIANT*,DispatchEx**);

typedef enum {
    NO_HINT,
    HINT_STRING,
    HINT_NUMBER
} hint_t;

HRESULT to_primitive(script_ctx_t*,VARIANT*,jsexcept_t*,VARIANT*, hint_t);
HRESULT to_boolean(VARIANT*,VARIANT_BOOL*);
HRESULT to_number(script_ctx_t*,VARIANT*,jsexcept_t*,VARIANT*);
HRESULT to_integer(script_ctx_t*,VARIANT*,jsexcept_t*,VARIANT*);
HRESULT to_int32(script_ctx_t*,VARIANT*,jsexcept_t*,INT*);
HRESULT to_uint32(script_ctx_t*,VARIANT*,jsexcept_t*,DWORD*);
HRESULT to_string(script_ctx_t*,VARIANT*,jsexcept_t*,BSTR*);
HRESULT to_object(script_ctx_t*,VARIANT*,IDispatch**);

typedef struct named_item_t {
    IDispatch *disp;
    DWORD flags;
    LPWSTR name;

    struct named_item_t *next;
} named_item_t;

struct _script_ctx_t {
    LONG ref;

    SCRIPTSTATE state;
    exec_ctx_t *exec_ctx;
    named_item_t *named_items;
    IActiveScriptSite *site;
    IInternetHostSecurityManager *secmgr;
    DWORD safeopt;
    DWORD version;
    LCID lcid;

    jsheap_t tmp_heap;

    IDispatch *host_global;

    BSTR last_match;
    DWORD last_match_index;
    DWORD last_match_length;

    DispatchEx *global;
    DispatchEx *function_constr;
    DispatchEx *activex_constr;
    DispatchEx *array_constr;
    DispatchEx *bool_constr;
    DispatchEx *date_constr;
    DispatchEx *error_constr;
    DispatchEx *eval_error_constr;
    DispatchEx *range_error_constr;
    DispatchEx *reference_error_constr;
    DispatchEx *regexp_error_constr;
    DispatchEx *syntax_error_constr;
    DispatchEx *type_error_constr;
    DispatchEx *uri_error_constr;
    DispatchEx *number_constr;
    DispatchEx *object_constr;
    DispatchEx *regexp_constr;
    DispatchEx *string_constr;
};

void script_release(script_ctx_t*);

static inline void script_addref(script_ctx_t *ctx)
{
    ctx->ref++;
}

HRESULT init_global(script_ctx_t*);
HRESULT init_function_constr(script_ctx_t*,DispatchEx*);
HRESULT create_object_prototype(script_ctx_t*,DispatchEx**);

HRESULT create_activex_constr(script_ctx_t*,DispatchEx**);
HRESULT create_array_constr(script_ctx_t*,DispatchEx*,DispatchEx**);
HRESULT create_bool_constr(script_ctx_t*,DispatchEx*,DispatchEx**);
HRESULT create_date_constr(script_ctx_t*,DispatchEx*,DispatchEx**);
HRESULT init_error_constr(script_ctx_t*,DispatchEx*);
HRESULT create_number_constr(script_ctx_t*,DispatchEx*,DispatchEx**);
HRESULT create_object_constr(script_ctx_t*,DispatchEx*,DispatchEx**);
HRESULT create_regexp_constr(script_ctx_t*,DispatchEx*,DispatchEx**);
HRESULT create_string_constr(script_ctx_t*,DispatchEx*,DispatchEx**);

IUnknown *create_ax_site(script_ctx_t*);

typedef struct {
    const WCHAR *str;
    DWORD len;
} match_result_t;

#define REM_CHECK_GLOBAL   0x0001
#define REM_RESET_INDEX    0x0002
#define REM_NO_CTX_UPDATE  0x0004
HRESULT regexp_match_next(script_ctx_t*,DispatchEx*,DWORD,const WCHAR*,DWORD,const WCHAR**,match_result_t**,
        DWORD*,DWORD*,match_result_t*);
HRESULT regexp_match(script_ctx_t*,DispatchEx*,const WCHAR*,DWORD,BOOL,match_result_t**,DWORD*);
HRESULT parse_regexp_flags(const WCHAR*,DWORD,DWORD*);
HRESULT regexp_string_match(script_ctx_t*,DispatchEx*,BSTR,VARIANT*,jsexcept_t*);

static inline VARIANT *get_arg(DISPPARAMS *dp, DWORD i)
{
    return dp->rgvarg + dp->cArgs-i-1;
}

static inline DWORD arg_cnt(const DISPPARAMS *dp)
{
    return dp->cArgs - dp->cNamedArgs;
}

static inline BOOL is_class(DispatchEx *jsdisp, jsclass_t class)
{
    return jsdisp->builtin_info->class == class;
}

static inline BOOL is_vclass(vdisp_t *vdisp, jsclass_t class)
{
    return is_jsdisp(vdisp) && is_class(vdisp->u.jsdisp, class);
}

static inline BOOL is_num_vt(enum VARENUM vt)
{
    return vt == VT_I4 || vt == VT_R8;
}

static inline DOUBLE num_val(const VARIANT *v)
{
    return V_VT(v) == VT_I4 ? V_I4(v) : V_R8(v);
}

static inline void num_set_val(VARIANT *v, DOUBLE d)
{
    if(d == (DOUBLE)(INT)d) {
        V_VT(v) = VT_I4;
        V_I4(v) = d;
    }else {
        V_VT(v) = VT_R8;
        V_R8(v) = d;
    }
}

static inline void num_set_nan(VARIANT *v)
{
    V_VT(v) = VT_R8;
#ifdef NAN
    V_R8(v) = NAN;
#else
    V_UI8(v) = (ULONGLONG)0x7ff80000<<32;
#endif
}

static inline DOUBLE ret_nan(void)
{
    VARIANT v;
    num_set_nan(&v);
    return V_R8(&v);
}

static inline void num_set_inf(VARIANT *v, BOOL positive)
{
    V_VT(v) = VT_R8;
#ifdef INFINITY
    V_R8(v) = positive ? INFINITY : -INFINITY;
#else
    V_UI8(v) = (ULONGLONG)0x7ff00000<<32;
    if(!positive)
        V_R8(v) = -V_R8(v);
#endif
}

static inline DWORD make_grfdex(script_ctx_t *ctx, DWORD flags)
{
    return (ctx->version << 28) | flags;
}

const char *debugstr_variant(const VARIANT*);

HRESULT WINAPI JScriptFactory_CreateInstance(IClassFactory*,IUnknown*,REFIID,void**);

extern LONG module_ref;

static inline void lock_module(void)
{
    InterlockedIncrement(&module_ref);
}

static inline void unlock_module(void)
{
    InterlockedDecrement(&module_ref);
}

static inline void *heap_alloc(size_t len)
{
    return HeapAlloc(GetProcessHeap(), 0, len);
}

static inline void *heap_alloc_zero(size_t len)
{
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len);
}

static inline void *heap_realloc(void *mem, size_t len)
{
    return HeapReAlloc(GetProcessHeap(), 0, mem, len);
}

static inline BOOL heap_free(void *mem)
{
    return HeapFree(GetProcessHeap(), 0, mem);
}

static inline LPWSTR heap_strdupW(LPCWSTR str)
{
    LPWSTR ret = NULL;

    if(str) {
        DWORD size;

        size = (strlenW(str)+1)*sizeof(WCHAR);
        ret = heap_alloc(size);
        memcpy(ret, str, size);
    }

    return ret;
}

#define DEFINE_THIS(cls,ifc,iface) ((cls*)((BYTE*)(iface)-offsetof(cls,lp ## ifc ## Vtbl)))
