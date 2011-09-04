/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the w64 mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within this package.
 */

#pragma once
#ifndef _INCLUDED_MM2
#define _INCLUDED_MM2

#include <crtdefs.h>
#include <mmintrin.h>

typedef union _DECLSPEC_INTRIN_TYPE _CRT_ALIGN(16) __m128
{
    float m128_f32[4];
    unsigned __int64 m128_u64[2];
    __int8 m128_i8[16];
    __int16 m128_i16[8];
    __int32 m128_i32[4];
    __int64 m128_i64[2];
    unsigned __int8 m128_u8[16];
    unsigned __int16 m128_u16[8];
    unsigned __int32 m128_u32[4];
} __m128;


extern __m128 _mm_load_ss(float const*);
extern int _mm_cvt_ss2si(__m128);

/* Alternate names */
#define _mm_cvtss_si32 _mm_cvt_ss2si


#endif /* _INCLUDED_MM2 */
