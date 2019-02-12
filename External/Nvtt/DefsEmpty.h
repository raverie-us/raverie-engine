// This code is in the public domain -- Ignacio Castaño <castano@gmail.com>

#ifndef NV_CORE_H
#error "Do not include this file directly."
#endif

// Function linkage
#define DLL_IMPORT
#define DLL_EXPORT
#define DLL_EXPORT_CLASS DLL_EXPORT

// Function calling modes
#define NV_CDECL
#define NV_STDCALL
#define NV_FASTCALL
#define NV_DEPRECATED

#define NV_PURE
#define NV_CONST

#define NV_NOINLINE
#define NV_FORCEINLINE inline

#define NV_THREAD_LOCAL

#include <stdint.h>
#include <intrin.h>

#if !defined __FUNC__
#define __FUNC__ __FUNCTION__ 
#endif

#define restrict    __restrict__

int strcasecmp(const void *s1, const void *s2);
