/*
 * Copyright 2008 Jacek Caban for CodeWeavers
 * Copyright 2009 Piotr Caban
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

#include <math.h>
#include <limits.h>

#include "jscript.h"
#include "ntsecapi.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(jscript);

static const WCHAR EW[] = {'E',0};
static const WCHAR LOG2EW[] = {'L','O','G','2','E',0};
static const WCHAR LOG10EW[] = {'L','O','G','1','0','E',0};
static const WCHAR LN2W[] = {'L','N','2',0};
static const WCHAR LN10W[] = {'L','N','1','0',0};
static const WCHAR PIW[] = {'P','I',0};
static const WCHAR SQRT2W[] = {'S','Q','R','T','2',0};
static const WCHAR SQRT1_2W[] = {'S','Q','R','T','1','_','2',0};
static const WCHAR absW[] = {'a','b','s',0};
static const WCHAR acosW[] = {'a','c','o','s',0};
static const WCHAR asinW[] = {'a','s','i','n',0};
static const WCHAR atanW[] = {'a','t','a','n',0};
static const WCHAR atan2W[] = {'a','t','a','n','2',0};
static const WCHAR ceilW[] = {'c','e','i','l',0};
static const WCHAR cosW[] = {'c','o','s',0};
static const WCHAR expW[] = {'e','x','p',0};
static const WCHAR floorW[] = {'f','l','o','o','r',0};
static const WCHAR logW[] = {'l','o','g',0};
static const WCHAR maxW[] = {'m','a','x',0};
static const WCHAR minW[] = {'m','i','n',0};
static const WCHAR powW[] = {'p','o','w',0};
static const WCHAR randomW[] = {'r','a','n','d','o','m',0};
static const WCHAR roundW[] = {'r','o','u','n','d',0};
static const WCHAR sinW[] = {'s','i','n',0};
static const WCHAR sqrtW[] = {'s','q','r','t',0};
static const WCHAR tanW[] = {'t','a','n',0};

static HRESULT math_constant(DOUBLE val, WORD flags, VARIANT *retv)
{
    switch(flags) {
    case DISPATCH_PROPERTYGET:
        V_VT(retv) = VT_R8;
        V_R8(retv) = val;
        return S_OK;
    case DISPATCH_PROPERTYPUT:
        return S_OK;
    }

    FIXME("unhandled flags %x\n", flags);
    return E_NOTIMPL;
}

/* ECMA-262 3rd Edition    15.8.1.1 */
static HRESULT Math_E(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    TRACE("\n");
    return math_constant(M_E, flags, retv);
}

/* ECMA-262 3rd Edition    15.8.1.4 */
static HRESULT Math_LOG2E(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    TRACE("\n");
    return math_constant(M_LOG2E, flags, retv);
}

/* ECMA-262 3rd Edition    15.8.1.4 */
static HRESULT Math_LOG10E(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    TRACE("\n");
    return math_constant(M_LOG10E, flags, retv);
}

static HRESULT Math_LN2(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    TRACE("\n");
    return math_constant(M_LN2, flags, retv);
}

static HRESULT Math_LN10(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    TRACE("\n");
    return math_constant(M_LN10, flags, retv);
}

/* ECMA-262 3rd Edition    15.8.1.6 */
static HRESULT Math_PI(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    TRACE("\n");
    return math_constant(M_PI, flags, retv);
}

static HRESULT Math_SQRT2(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    TRACE("\n");
    return math_constant(M_SQRT2, flags, retv);
}

static HRESULT Math_SQRT1_2(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    TRACE("\n");
    return math_constant(M_SQRT1_2, flags, retv);
}

/* ECMA-262 3rd Edition    15.8.2.12 */
static HRESULT Math_abs(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT v;
    DOUBLE d;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        if(retv)
            num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    d = num_val(&v);
    if(retv)
        num_set_val(retv, d < 0.0 ? -d : d);
    return S_OK;
}

static HRESULT Math_acos(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT v;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        if(retv) num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    if(retv) num_set_val(retv, acos(num_val(&v)));
    return S_OK;
}

static HRESULT Math_asin(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT v;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        if(retv) num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    if(retv) num_set_val(retv, asin(num_val(&v)));
    return S_OK;
}

static HRESULT Math_atan(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT v;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        if(retv) num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    if(retv) num_set_val(retv, atan(num_val(&v)));
    return S_OK;
}

static HRESULT Math_atan2(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT v1, v2;
    HRESULT hres;

    TRACE("\n");

    if(arg_cnt(dp)<2) {
        if(retv) num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v1);
    if(FAILED(hres))
        return hres;

    hres = to_number(ctx, get_arg(dp, 1), ei, &v2);
    if(FAILED(hres))
        return hres;

    if(retv) num_set_val(retv, atan2(num_val(&v1), num_val(&v2)));
    return S_OK;
}

/* ECMA-262 3rd Edition    15.8.2.6 */
static HRESULT Math_ceil(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT v;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        if(retv)
            num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    if(retv)
        num_set_val(retv, ceil(num_val(&v)));
    return S_OK;
}

static HRESULT Math_cos(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT v;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        if(retv) num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    if(retv) num_set_val(retv, cos(num_val(&v)));
    return S_OK;
}

static HRESULT Math_exp(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT v;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        if(retv) num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    if(retv) num_set_val(retv, exp(num_val(&v)));
    return S_OK;
}

static HRESULT Math_floor(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT v;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        if(retv)
            num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    if(retv)
        num_set_val(retv, floor(num_val(&v)));
    return S_OK;
}

static HRESULT Math_log(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT v;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        if(retv)
            num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    if(retv)
        num_set_val(retv, log(num_val(&v)));
    return S_OK;
}

/* ECMA-262 3rd Edition    15.8.2.11 */
static HRESULT Math_max(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    DOUBLE max, d;
    VARIANT v;
    DWORD i;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        if(retv)
            num_set_inf(retv, FALSE);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    max = num_val(&v);
    for(i=1; i < arg_cnt(dp); i++) {
        hres = to_number(ctx, get_arg(dp, i), ei, &v);
        if(FAILED(hres))
            return hres;

        d = num_val(&v);
        if(d > max || isnan(d))
            max = d;
    }

    if(retv)
        num_set_val(retv, max);
    return S_OK;
}

/* ECMA-262 3rd Edition    15.8.2.12 */
static HRESULT Math_min(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    DOUBLE min, d;
    VARIANT v;
    DWORD i;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        if(retv)
            num_set_inf(retv, TRUE);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    min = num_val(&v);
    for(i=1; i < arg_cnt(dp); i++) {
        hres = to_number(ctx, get_arg(dp, i), ei, &v);
        if(FAILED(hres))
            return hres;

        d = num_val(&v);
        if(d < min || isnan(d))
            min = d;
    }

    if(retv)
        num_set_val(retv, min);
    return S_OK;
}

/* ECMA-262 3rd Edition    15.8.2.13 */
static HRESULT Math_pow(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT x, y;
    HRESULT hres;

    TRACE("\n");

    if(arg_cnt(dp) < 2) {
        if(retv) num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &x);
    if(FAILED(hres))
        return hres;

    hres = to_number(ctx, get_arg(dp, 1), ei, &y);
    if(FAILED(hres))
        return hres;

    if(retv)
        num_set_val(retv, pow(num_val(&x), num_val(&y)));
    return S_OK;
}

/* ECMA-262 3rd Edition    15.8.2.14 */
static HRESULT Math_random(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    UINT r;

    TRACE("\n");

    if(!RtlGenRandom(&r, sizeof(r)))
        return E_UNEXPECTED;

    if(retv)
        num_set_val(retv, (DOUBLE)r/(DOUBLE)UINT_MAX);

    return S_OK;
}

/* ECMA-262 3rd Edition    15.8.2.15 */
static HRESULT Math_round(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT v;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    if(retv)
        num_set_val(retv, floor(num_val(&v)+0.5));
    return S_OK;
}

static HRESULT Math_sin(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT v;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        if(retv) num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    if(retv) num_set_val(retv, sin(num_val(&v)));
    return S_OK;
}

static HRESULT Math_sqrt(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT v;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        if(retv) num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    if(retv) num_set_val(retv, sqrt(num_val(&v)));
    return S_OK;
}

static HRESULT Math_tan(script_ctx_t *ctx, vdisp_t *jsthis, WORD flags, DISPPARAMS *dp,
        VARIANT *retv, jsexcept_t *ei, IServiceProvider *sp)
{
    VARIANT v;
    HRESULT hres;

    TRACE("\n");

    if(!arg_cnt(dp)) {
        if(retv) num_set_nan(retv);
        return S_OK;
    }

    hres = to_number(ctx, get_arg(dp, 0), ei, &v);
    if(FAILED(hres))
        return hres;

    if(retv) num_set_val(retv, tan(num_val(&v)));
    return S_OK;
}

static const builtin_prop_t Math_props[] = {
    {EW,        Math_E,        0},
    {LN10W,     Math_LN10,     0},
    {LN2W,      Math_LN2,      0},
    {LOG10EW,   Math_LOG10E,   0},
    {LOG2EW,    Math_LOG2E,    0},
    {PIW,       Math_PI,       0},
    {SQRT1_2W,  Math_SQRT1_2,  0},
    {SQRT2W,    Math_SQRT2,    0},
    {absW,      Math_abs,      PROPF_METHOD|1},
    {acosW,     Math_acos,     PROPF_METHOD|1},
    {asinW,     Math_asin,     PROPF_METHOD|1},
    {atanW,     Math_atan,     PROPF_METHOD|1},
    {atan2W,    Math_atan2,    PROPF_METHOD|2},
    {ceilW,     Math_ceil,     PROPF_METHOD|1},
    {cosW,      Math_cos,      PROPF_METHOD|1},
    {expW,      Math_exp,      PROPF_METHOD|1},
    {floorW,    Math_floor,    PROPF_METHOD|1},
    {logW,      Math_log,      PROPF_METHOD|1},
    {maxW,      Math_max,      PROPF_METHOD|2},
    {minW,      Math_min,      PROPF_METHOD|2},
    {powW,      Math_pow,      PROPF_METHOD|2},
    {randomW,   Math_random,   PROPF_METHOD},
    {roundW,    Math_round,    PROPF_METHOD|1},
    {sinW,      Math_sin,      PROPF_METHOD|1},
    {sqrtW,     Math_sqrt,     PROPF_METHOD|1},
    {tanW,      Math_tan,      PROPF_METHOD|1}
};

static const builtin_info_t Math_info = {
    JSCLASS_MATH,
    {NULL, NULL, 0},
    sizeof(Math_props)/sizeof(*Math_props),
    Math_props,
    NULL,
    NULL
};

HRESULT create_math(script_ctx_t *ctx, DispatchEx **ret)
{
    DispatchEx *math;
    HRESULT hres;

    math = heap_alloc_zero(sizeof(DispatchEx));
    if(!math)
        return E_OUTOFMEMORY;

    hres = init_dispex_from_constr(math, ctx, &Math_info, ctx->object_constr);
    if(FAILED(hres)) {
        heap_free(math);
        return hres;
    }

    *ret = math;
    return S_OK;
}
